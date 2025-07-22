# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "F:/programmes/Espressif/v5.3.1/esp-idf/components/bootloader/subproject"
  "E:/Documents/Programmation/midi_marble_sequencer/software/midi_marble_sequencer_main/build/bootloader"
  "E:/Documents/Programmation/midi_marble_sequencer/software/midi_marble_sequencer_main/build/bootloader-prefix"
  "E:/Documents/Programmation/midi_marble_sequencer/software/midi_marble_sequencer_main/build/bootloader-prefix/tmp"
  "E:/Documents/Programmation/midi_marble_sequencer/software/midi_marble_sequencer_main/build/bootloader-prefix/src/bootloader-stamp"
  "E:/Documents/Programmation/midi_marble_sequencer/software/midi_marble_sequencer_main/build/bootloader-prefix/src"
  "E:/Documents/Programmation/midi_marble_sequencer/software/midi_marble_sequencer_main/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "E:/Documents/Programmation/midi_marble_sequencer/software/midi_marble_sequencer_main/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "E:/Documents/Programmation/midi_marble_sequencer/software/midi_marble_sequencer_main/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
