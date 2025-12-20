#include "PresetDisplay.h"
#include "MidiMapperPlugin.h"
#include "MidiMapping.h"

PresetDisplay::PresetDisplay (MidiMapperPluginProcessor& ownerIn) : owner (ownerIn)
{
    setInterceptsMouseClicks (true, true);
    setMouseClickGrabsKeyboardFocus (false);
    setOpaque (true);

    keyboardComponent = std::make_unique<juce::MidiKeyboardComponent> (keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard);
    addAndMakeVisible (keyboardComponent.get());
    keyboardComponent->setEnabled (false);

    keyboardState.addListener (this);

    for (int i = 0; i < (int) channelButtons.size(); ++i)
    {
        channelButtons[(size_t) i].setButtonText (juce::String (i + 1));
        channelButtons[(size_t) i].setClickingTogglesState (true);
        addAndMakeVisible (channelButtons[(size_t) i]);
        channelButtons[(size_t) i].setEnabled (false);
        channelButtons[(size_t) i].setToggleState (false, juce::dontSendNotification);
        channelButtons[(size_t) i].onClick = [this, i]
        {
            if (selectedRow < 0) return;

            const auto inputNotes = MidiMapping::getDefaultInputNotes();
            const int rows = 8;
            const int cols = (int) inputNotes.size();
            if (selectedRow >= rows) return;

            auto& midiMapping = owner.getMidiMapping();
            const juce::ScopedLock sl (midiMapping.getLock());

            if (selectedCol >= 0)
            {
                const int inNote = inputNotes[(size_t) selectedCol];
                auto& m = midiMapping.getMapping()[selectedRow][inNote];
                m.mapped = true;
                m.numOuts = 1;
                m.outChannel[0] = i;
                if (m.outNote[0] < 0 || m.outNote[0] >= 128)
                    m.outNote[0] = inNote;
            }
            else
            {
                for (int col = 0; col < cols; ++col)
                {
                    const int inNote = inputNotes[(size_t) col];
                    auto& m = midiMapping.getMapping()[selectedRow][inNote];
                    m.mapped = true;
                    m.numOuts = 1;
                    m.outChannel[0] = i;
                    if (m.outNote[0] < 0 || m.outNote[0] >= 128)
                        m.outNote[0] = inNote;
                }
            }

            for (int j = 0; j < (int) channelButtons.size(); ++j)
                channelButtons[(size_t) j].setToggleState (j == i, juce::dontSendNotification);

            repaint();
        };
    }
    
    startTimer (33);
}

PresetDisplay::~PresetDisplay()
{
    stopTimer();
    keyboardState.removeListener (this);
}

bool PresetDisplay::pointToCell (juce::Point<int> p, int& outRow, int& outCol) const
{
    const auto notes = MidiMapping::getDefaultInputNotes();
    const int rows = 8;
    const int cols = (int) notes.size();

    juce::Rectangle<int> r = getLocalBounds();
    r.removeFromBottom (keyboardHeight);
    r.removeFromBottom (buttonRowHeight);

    const int rowH = r.getHeight() / rows;
    const int availableW = jmax (0, r.getWidth() - 24);
    const int colW = availableW / cols;

    if (p.y < r.getY() || p.y >= r.getY() + rows * rowH)
        return false;
    if (p.x < r.getX() + 24 || p.x >= r.getX() + 24 + cols * colW)
        return false;

    outRow = (p.y - r.getY()) / rowH;
    outCol = (p.x - (r.getX() + 24)) / colW;
    if (outRow < 0 || outRow >= rows || outCol < 0 || outCol >= cols)
        return false;
    return true;
}

void PresetDisplay::mouseDown (const juce::MouseEvent& e)
{
    int row, col;

    juce::Rectangle<int> r = getLocalBounds();
    r.removeFromBottom (keyboardHeight);
    r.removeFromBottom (buttonRowHeight);
    const int rows = 8;
    const int rowH = r.getHeight() / rows;

    const auto pos = e.getPosition();

    if (pos.x >= r.getX() && pos.x < r.getX() + 24 && pos.y >= r.getY() && pos.y < r.getY() + rows * rowH)
    {
        const int clickedRow = (pos.y - r.getY()) / rowH;
        selectedRow = clickedRow;
        selectedCol = -1;

        for (auto& b : channelButtons) b.setEnabled (true);
        if (keyboardComponent) keyboardComponent->setEnabled (false);

        int active = -1;
        {
            auto& midiMapping = owner.getMidiMapping();
            const juce::ScopedLock sl (midiMapping.getLock());
            const auto notes = MidiMapping::getDefaultInputNotes();
            for (int ni = 0; ni < (int) notes.size(); ++ni)
            {
                const auto& m = midiMapping.getMapping()[selectedRow][notes[(size_t) ni]];
                if (m.mapped && m.numOuts > 0) { active = m.outChannel[0]; break; }
            }
        }
        for (int j = 0; j < (int) channelButtons.size(); ++j)
            channelButtons[(size_t) j].setToggleState (j == active, juce::dontSendNotification);

        repaint();
        if (onCellSelected) onCellSelected (selectedRow, -1);
        return;
    }

    if (pointToCell (e.getPosition(), row, col))
    {
        selectedRow = row;
        selectedCol = col;
        if (keyboardComponent)
            keyboardComponent->setEnabled (true);
        for (auto& b : channelButtons) b.setEnabled (true);

        const auto inputNotes = MidiMapping::getDefaultInputNotes();
        const int inNote = inputNotes[(size_t) selectedCol];
        int active = -1;
        {
            auto& midiMapping = owner.getMidiMapping();
            const juce::ScopedLock sl (midiMapping.getLock());
            const auto& m = midiMapping.getMapping()[selectedRow][inNote];
            if (m.mapped && m.numOuts > 0)
                active = m.outChannel[0];
        }
        for (int j = 0; j < (int) channelButtons.size(); ++j)
            channelButtons[(size_t) j].setToggleState (j == active, juce::dontSendNotification);

        repaint();

        if (onCellSelected)
            onCellSelected (row, col);
    }
    else
    {
        selectedRow = -1;
        selectedCol = -1;
        if (keyboardComponent)
            keyboardComponent->setEnabled (false);
        for (auto& b : channelButtons) { b.setEnabled (false); b.setToggleState (false, juce::dontSendNotification); }
        repaint();
    }
}

void PresetDisplay::mouseDoubleClick (const juce::MouseEvent& e)
{
    int row, col;
    if (! pointToCell (e.getPosition(), row, col))
        return;

    selectedRow = row;
    selectedCol = col;
    if (keyboardComponent)
        keyboardComponent->setEnabled (true);
    for (auto& b : channelButtons) b.setEnabled (true);

    const auto inputNotes = MidiMapping::getDefaultInputNotes();
    const int inNote = inputNotes[(size_t) selectedCol];
    int active = -1;
    {
        auto& midiMapping = owner.getMidiMapping();
        const juce::ScopedLock sl (midiMapping.getLock());
        const auto& m = midiMapping.getMapping()[selectedRow][inNote];
        if (m.mapped && m.numOuts > 0)
            active = m.outChannel[0];
    }
    for (int j = 0; j < (int) channelButtons.size(); ++j)
        channelButtons[(size_t) j].setToggleState (j == active, juce::dontSendNotification);

    repaint();

    if (onCellDoubleClicked)
        onCellDoubleClicked (row, col);
}

void PresetDisplay::resized()
{
    auto bounds = getLocalBounds();
    juce::Rectangle<int> kbRect = bounds.removeFromBottom (keyboardHeight);

    juce::Rectangle<int> buttonRect = bounds.removeFromBottom (buttonRowHeight);
    const int pad = 4;
    const int totalButtonWidth = buttonRect.getWidth() - pad * 2;
    const int btnW = totalButtonWidth / (int) channelButtons.size();
    for (int i = 0; i < (int) channelButtons.size(); ++i)
    {
        channelButtons[(size_t) i].setBounds (buttonRect.getX() + pad + i * btnW, buttonRect.getY() + 2, btnW - 4, buttonRect.getHeight() - 4);
    }

    if (keyboardComponent)
        keyboardComponent->setBounds (kbRect.reduced (2));
}

void PresetDisplay::handleNoteOn (juce::MidiKeyboardState*, int /*midiChannel*/, int midiNoteNumber, float /*velocity*/)
{
    if (selectedRow < 0)
        return;

    const auto inputNotes = MidiMapping::getDefaultInputNotes();
    const int rows = 8;
    const int cols = (int) inputNotes.size();
    if (selectedRow >= rows) return;

    const int inNoteIndex = (selectedCol >= 0) ? selectedCol : 0;

    int active = -1;
    {
        auto& midiMapping = owner.getMidiMapping();
        const juce::ScopedLock sl (midiMapping.getLock());

        if (selectedCol >= 0)
        {
            auto& m = midiMapping.getMapping()[selectedRow][inputNotes[(size_t) selectedCol]];
            int existingOutChan = (m.numOuts > 0) ? m.outChannel[0] : 0;
            m.mapped = true;
            m.numOuts = 1;
            m.outChannel[0] = existingOutChan;
            m.outNote[0] = midiNoteNumber;
            active = m.outChannel[0];
        }
        else
        {
            for (int col = 0; col < cols; ++col)
            {
                auto& m = midiMapping.getMapping()[selectedRow][inputNotes[(size_t) col]];
                int existingOutChan = (m.numOuts > 0) ? m.outChannel[0] : 0;
                m.mapped = true;
                m.numOuts = 1;
                m.outChannel[0] = existingOutChan;
                m.outNote[0] = midiNoteNumber;
                active = m.outChannel[0];
            }
        }
    }

    for (int j = 0; j < (int) channelButtons.size(); ++j)
        channelButtons[(size_t) j].setToggleState (j == active, juce::dontSendNotification);

    if (selectedCol >= 0)
    {
        selectedCol += 1;
        if (selectedCol >= cols)
        {
            selectedCol = 0;
            if (selectedRow < rows - 1) selectedRow += 1;
        }

        int newActive = -1;
        {
            auto& midiMapping = owner.getMidiMapping();
            const juce::ScopedLock sl (midiMapping.getLock());
            const auto& m = midiMapping.getMapping()[selectedRow][inputNotes[(size_t) selectedCol]];
            if (m.mapped && m.numOuts > 0) newActive = m.outChannel[0];
        }
        for (int j = 0; j < (int) channelButtons.size(); ++j)
            channelButtons[(size_t) j].setToggleState (j == newActive, juce::dontSendNotification);

        if (onCellSelected) onCellSelected (selectedRow, selectedCol);
    }

    repaint();
}

void PresetDisplay::handleNoteOff (juce::MidiKeyboardState*, int, int, float) {}

juce::String getMidiNoteName(int note)
{
    int noteInOctave = note % 12;
    bool useSharps = true;

    if (noteInOctave == 3 || noteInOctave == 8 || noteInOctave == 10)
        useSharps = false;
    
    return juce::MidiMessage::getMidiNoteName(note, useSharps, true, 3);
}

void PresetDisplay::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId).darker (0.03f));

    const auto notes = MidiMapping::getDefaultInputNotes();
    const std::array<juce::Colour, 5> colours = {
        juce::Colour (0xFF3f2e2e),
        juce::Colour (0xFFe96342),
        juce::Colour (0xFF86a830),
        juce::Colour (0xFF52a7b9),
        juce::Colours::white
    };

    const int rows = 8;
    const int cols = (int) notes.size();

    juce::Rectangle<int> r = getLocalBounds();
    r.removeFromBottom (keyboardHeight);
    r.removeFromBottom (buttonRowHeight);

    const int rowH = r.getHeight() / rows;
    const int availableW = jmax (0, r.getWidth() - 24);
    const int colW = availableW / cols;

    int circleDiameter = jmin(colW, rowH) * 2 / 3;
    circleDiameter = jmin (circleDiameter, jmin (colW, rowH));
    const int radius = circleDiameter / 2;

    juce::Font font = g.getCurrentFont();
    font.setHeight (jmin (24.0f, (float) rowH));
    g.setFont (font);

    std::array<int, 8> primaryOut; primaryOut.fill (-1);
    {
        auto& midiMapping = owner.getMidiMapping();
        const juce::ScopedLock sl (midiMapping.getLock());
        for (int ch = 0; ch < rows; ++ch)
        {
            for (int ni = 0; ni < cols; ++ni)
            {
                const int note = notes[(size_t) ni];
                const auto& m = midiMapping.getMapping()[ch][note];
                if (m.mapped && m.numOuts > 0)
                {
                    primaryOut[ch] = m.outChannel[0];
                    break;
                }
            }
        }
    }

    int groupStart = -1;
    for (int ch = 0; ch <= rows; ++ch)
    {
        const int cur = (ch < rows) ? primaryOut[ch] : -2;
        const int prev = (ch > 0) ? primaryOut[ch - 1] : -3;

        if (ch == 0)
            groupStart = 0;

        if (ch > 0 && cur != prev)
        {
            const int gStart = groupStart;
            const int gEnd = ch - 1;

            if (primaryOut[gStart] >= 0)
            {
                const int y1 = r.getY() + gStart * rowH + 1;
                const int y2 = r.getY() + (gEnd + 1) * rowH - 1;
                const int x1 = r.getX() + 24 - 6;
                // reduce width to stay within bounds: subtract border thickness and margin
                const int w = availableW + 12 - 8;

                const float normChan = juce::jlimit (0.0f, 1.0f, primaryOut[gStart] / 7.0f);
                const float greyLevel = 0.2f + normChan * 0.75f;
                const juce::Colour groupBg = juce::Colour::greyLevel (greyLevel).withAlpha (0.12f);

                g.setColour (groupBg);
                g.fillRect (x1, y1, w, (y2 - y1 + 1));

                const juce::Colour borderColour = juce::Colours::white.withAlpha (0.5f);
                g.setColour (borderColour);
                g.drawRoundedRectangle ((float)x1, (float)y1, (float)w, (float)(y2 - y1 + 1), 4.0f, 2.0f);
            }

            groupStart = ch;
        }
    }

    for (int ch = 0; ch < rows; ++ch)
    {
        const int y = r.getY() + ch * rowH;
        const juce::Rectangle<int> rowRect (r.getX(), y, r.getWidth(), rowH);
        g.setColour (juce::Colours::grey.withAlpha (0.15f));
        g.fillRect (rowRect);

        g.setColour (juce::Colours::lightgrey);
        auto rowLabelRect = rowRect;
        rowLabelRect = rowLabelRect.removeFromLeft (20).reduced (2);
        g.drawText (juce::String (ch), rowLabelRect, juce::Justification::centredLeft);

        for (int ni = 0; ni < cols; ++ni)
        {
            const int x = r.getX() + ni * colW + 24;
            const int contentLeft = x + 20;
            const int cx = contentLeft + radius;
            const int cy = y + rowH / 2;

            bool mapped = false;
            int outChan = -1;
            int outNote = -1;
            bool isActive = false;
            {
                auto& midiMapping = owner.getMidiMapping();
                const juce::ScopedLock sl (midiMapping.getLock());
                const int note = notes[(size_t) ni];
                const auto& m = midiMapping.getMapping()[ch][note];
                if (m.mapped && m.numOuts > 0)
                {
                    mapped = true;
                    outChan = m.outChannel[0];
                    outNote = m.outNote[0];
                    isActive = midiMapping.isMappingActive (ch, note);
                }
            }

            // selection overlay: highlight either single cell or whole row
            if (selectedRow == ch && (selectedCol == ni || selectedCol == -1))
            {
                g.setColour (juce::Colours::yellow.withAlpha (0.25f));
                g.fillRect (x - 4, y + 2, colW, rowH - 4);
            }
            
            // active mapping highlight: draw blue highlight
            if (isActive)
            {
                g.setColour (juce::Colours::blue.withAlpha (0.3f));
                g.fillRect (x - 4, y + 2, colW, rowH - 4);
            }

            if (mapped)
            {
                // draw circle with input-note color
                g.setColour (colours[(size_t) ni]);
                g.fillEllipse ((float) (cx - radius), (float) (cy - radius), (float) circleDiameter, (float) circleDiameter);
                g.setColour (juce::Colours::black.withAlpha (0.35f));
                g.drawEllipse ((float) (cx - radius), (float) (cy - radius), (float) circleDiameter, (float) circleDiameter, 1.0f);

                const juce::String noteName = getMidiNoteName (outNote);
                const juce::String txt = juce::String (outChan + 1) + " " + noteName;

                const int primary = primaryOut[ch];
                float greyLevel = 0.5f;
                if (primary >= 0)
                    greyLevel = 0.2f + juce::jlimit (0.0f, 1.0f, primary / 15.0f) * 0.75f;

                const juce::Colour textColour = (greyLevel > 0.6f) ? juce::Colours::black.withAlpha (0.9f) : juce::Colours::white.withAlpha (0.9f);

                g.setColour (textColour);
                const int textX = contentLeft + circleDiameter + 8;
                const int textW = x + colW - textX - 12;
                const juce::Rectangle<int> textArea (textX, y, juce::jmax (0, textW), rowH);
                g.drawText (txt, textArea, juce::Justification::centredLeft);
            }
            else
            {
                g.setColour (juce::Colours::grey.withAlpha (0.12f));
                g.drawEllipse ((float) (cx - radius), (float) (cy - radius), (float) circleDiameter, (float) circleDiameter, 1.0f);
            }
        }
    }
}

void PresetDisplay::timerCallback()
{
    repaint();
}
