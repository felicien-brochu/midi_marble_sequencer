#include "Editor.h"
#include "MidiMapperPlugin.h"

// small modal dialog to enter new preset name
struct NewPresetDialog  : public juce::DialogWindow
{
    NewPresetDialog() : DialogWindow ("New preset",
                                      juce::Colours::lightgrey,
                                      true)
    {
        addAndMakeVisible (&label);
        label.setText ("preset name:", juce::dontSendNotification);
        addAndMakeVisible (&editor);
        editor.setText ("");

        addAndMakeVisible (&okButton);
        okButton.setButtonText ("Ok");
        addAndMakeVisible (&cancelButton);
        cancelButton.setButtonText ("Cancel");

        okButton.onClick = [this]
        {
            if (onOk) onOk (editor.getText());
            closeDialog();
        };

        cancelButton.onClick = [this]
        {
            if (onCancel) onCancel();
            closeDialog();
        };

        setUsingNativeTitleBar (true);
        setAlwaysOnTop (true);
        setResizable (false, false);
        centreWithSize (420, 120);
        setVisible (true);
    }

    std::function<void (const juce::String&)> onOk;
    std::function<void()> onCancel;

    void closeDialog()
    {
        setVisible (false);
        removeFromDesktop();
        delete this;
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced (8);
        auto row = area.removeFromTop (20);
        label.setBounds (row.removeFromLeft (100));
        editor.setBounds (area.removeFromTop (24));
        auto buttons = area.removeFromTop (32);
        cancelButton.setBounds (buttons.removeFromRight (80).reduced (4));
        okButton.setBounds (buttons.removeFromRight (80).reduced (4));
    }

private:
    juce::Label label { {}, {} };
    juce::TextEditor editor;
    juce::TextButton okButton { "Ok" };
    juce::TextButton cancelButton { "Cancel" };
};

struct ConfirmDialog : public juce::DialogWindow
{
    ConfirmDialog(const juce::String& title, const juce::String& message)
        : DialogWindow (title, juce::Colours::lightgrey, true)
    {
        addAndMakeVisible (&label);
        label.setText (message, juce::dontSendNotification);

        addAndMakeVisible (&okButton);
        okButton.setButtonText ("Ok");
        addAndMakeVisible (&cancelButton);
        cancelButton.setButtonText ("Cancel");

        okButton.onClick = [this] { if (onResult) onResult (true); closeDialog(); };
        cancelButton.onClick = [this] { if (onResult) onResult (false); closeDialog(); };

        setUsingNativeTitleBar (true);
        setAlwaysOnTop (true);
        setResizable (false, false);
        centreWithSize (420, 120);
        setVisible (true);
    }

    std::function<void (bool)> onResult;

    void closeDialog()
    {
        setVisible (false);
        removeFromDesktop();
        delete this;
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced (8);
        auto row = area.removeFromTop (20);
        label.setBounds (area.removeFromTop (24));
        auto buttons = area.removeFromTop (32);
        cancelButton.setBounds (buttons.removeFromRight (80).reduced (4));
        okButton.setBounds (buttons.removeFromRight (80).reduced (4));
    }

private:
    juce::Label label { {}, {} };
    juce::TextButton okButton { "Ok" };
    juce::TextButton cancelButton { "Cancel" };
};

static bool isValidFilename (const juce::String& name)
{
    if (name.isEmpty()) return false;
    // disallow characters: \ / : * ? " < > | and control chars
    const juce::String invalid = "\\/:*?\"<>|";
    for (auto c : name)
    {
        if (c < 32) return false;
        if (invalid.containsChar (c)) return false;
    }
    return true;
}

Editor::Editor (MidiMapperPluginProcessor& ownerIn)
    : AudioProcessorEditor (ownerIn), owner (ownerIn), presetDisplay (ownerIn)
{
    addAndMakeVisible (presetsBox);
    addAndMakeVisible (presetDisplay);
    addAndMakeVisible (newButton);
    addAndMakeVisible (saveButton);

    setResizable (true, true);
    lastUIWidth .referTo (owner.state.getChildWithName ("uiState").getPropertyAsValue ("width",  nullptr));
    lastUIHeight.referTo (owner.state.getChildWithName ("uiState").getPropertyAsValue ("height", nullptr));
    selectedPreset.referTo (owner.state.getChildWithName ("uiState").getPropertyAsValue ("selectedPreset", nullptr));
    setSize (lastUIWidth.getValue(), lastUIHeight.getValue());

    lastUIWidth. addListener (this);
    lastUIHeight.addListener (this);

    presetsBox.onChange = [&]
    {
        const auto presetIndex = presetsBox.getSelectedItemIndex();
        if (presetIndex > 0)
            selectedPreset = presetsBox.getItemText(presetIndex);
        else
            selectedPreset = String();
        onSelectedPresetChanged();
    };

    newButton.onClick = [&]
    {
        // show dialog and handle creation asynchronously via callback
        auto* dlg = new NewPresetDialog();
        dlg->onOk = [this] (const juce::String& input)
        {
            String name = input.trim();
            if (name.isEmpty())
                return;

            if (! isValidFilename (name))
            {
                juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon, "Invalid name", "The preset name contains invalid characters.");
                return;
            }

            if (! name.endsWithIgnoreCase (".xml"))
                name += ".xml";

            const File dir (MAPPING_PRESETS_PATH);
            if (! dir.exists()) dir.createDirectory();
            const File presetFile = dir.getChildFile (name);

            // creation logic captured for reuse if overwrite confirmed
            auto doCreate = [this, name]
            {
                // If the name already exists in combobox (in-memory), select it, otherwise add it
                int foundId = -1;
                for (int i = 1; i <= presetsBox.getNumItems(); ++i)
                    if (presetsBox.getItemText(i) == name)
                        foundId = i;

                if (foundId < 0)
                {
                    const int newId = presetsBox.getNumItems() + 1;
                    presetsBox.addItem (name, newId);
                    presetsBox.setSelectedId (newId, dontSendNotification);
                }
                else
                {
                    presetsBox.setSelectedId (foundId, dontSendNotification);
                }

                selectedPreset = name;

                // create default mapping in memory (do not write file yet)
                owner.getMidiMapping().createDefaultMapping();

                // notify display
                presetDisplay.repaint();

                // make selection in UI
                onSelectedPresetChanged();
            };

            if (presetFile.existsAsFile())
            {
                // ask to confirm overwrite on save; we still won't write now, just confirm creation in memory
                auto* cdlg = new ConfirmDialog ("Preset exists", "A preset file with that name already exists. Create and overwrite when saving?");
                cdlg->onResult = [doCreate] (bool ok) { if (ok) doCreate(); };
            }
            else
            {
                doCreate();
            }
        };

        dlg->onCancel = [](){};
    };

    saveButton.onClick = [&]
    {
        const auto presetName = selectedPreset.toString();
        if (presetName.isEmpty())
            return; // nothing to save for 'none' or empty

        const File presetFile = File (MAPPING_PRESETS_PATH).getChildFile (presetName);

        XmlElement mappingXml ("mapping");
        owner.getMidiMapping().saveToXml (mappingXml);

        // write to disk now
        mappingXml.writeTo (presetFile);
        // ensure presets list reflects file on disk
        populatePresets();

        // reselect saved preset and notify change so the UI stays consistent
        int foundIndex = -1;
        for (int i = 0; i < presetsBox.getNumItems(); ++i)
            if (presetsBox.getItemText(i) == presetName) { foundIndex = i; break; }

        if (foundIndex >= 0)
            presetsBox.setSelectedItemIndex (foundIndex, juce::dontSendNotification);

        // keep the saved preset as the active selection in state and UI without reloading from disk
        selectedPreset = presetName;
    };

    populatePresets();

    const auto saved = selectedPreset.toString();
    if (saved.isNotEmpty())
    {
        for (int i = 1; i <= presetsBox.getNumItems(); ++i)
            if (presetsBox.getItemText(i) == saved)
                presetsBox.setSelectedId (i, dontSendNotification);
    }
    else if (presetsBox.getNumItems() > 0)
        presetsBox.setSelectedId (1);

    presetDisplay.repaint();
}

void Editor::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void Editor::resized()
{
    auto bounds = getLocalBounds();
    auto topArea = bounds.removeFromTop (30);

    // reserve space on the right for the new and save buttons
    auto buttonsArea = topArea.removeFromRight (160);
    auto saveArea = buttonsArea.removeFromRight (80);
    auto newArea = buttonsArea; // remaining left half
    saveButton.setBounds (saveArea.withTrimmedLeft (4).withTrimmedRight (4).withSizeKeepingCentre (60, 24));
    newButton.setBounds (newArea.withTrimmedLeft (4).withTrimmedRight (4).withSizeKeepingCentre (60, 24));

    presetsBox.setBounds (topArea.withTrimmedLeft (4).withTrimmedRight (4).withSizeKeepingCentre (300, 24));

    // PresetDisplay fills the remaining space
    presetDisplay.setBounds (bounds);

    lastUIWidth  = getWidth();
    lastUIHeight = getHeight();
}

void Editor::onSelectedPresetChanged()
{
    const auto presetName = selectedPreset.toString();
    if (presetName == "none" || presetName.isEmpty())
    {
        owner.clearMapping();
        presetDisplay.repaint();
        return;
    }

    const File presetFile = File (MAPPING_PRESETS_PATH).getChildFile (presetName);
    if (presetFile.existsAsFile())
    {
        if (auto xml = parseXML (presetFile))
        {
            if (xml->hasTagName ("mapping"))
            {
                owner.loadMappingFromXml (xml.get());
                presetDisplay.repaint();
            }
        }
    }
}

void Editor::valueChanged (Value& value)
{
    setSize (lastUIWidth.getValue(), lastUIHeight.getValue());
}

void Editor::populatePresets()
{
    presetsBox.clear (false);

    const File dir (MAPPING_PRESETS_PATH);
    if (! dir.exists()) dir.createDirectory();

    presetsBox.addItem("none", 1);
    if (dir.isDirectory())
    {
        DirectoryIterator it (dir, false, "*.xml");
        int id = 2;
        while (it.next())
        {
            const File f = it.getFile();
            if (f.existsAsFile())
            {
                presetsBox.addItem (f.getFileName(), id);
                ++id;
            }
        }
    }
}
