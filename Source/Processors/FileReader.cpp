/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "FileReader.h"
#include "Editors/FileReaderEditor.h"
#include <stdio.h>

FileReader::FileReader()
    : GenericProcessor("File Reader")
{

	input = 0;
	timestamp = 0; 

	enabledState(false);

}

FileReader::~FileReader()
{
	if (input)
        fclose(input);
}

AudioProcessorEditor* FileReader::createEditor()
{
	editor = new FileReaderEditor(this, true);

	return editor;

}


float FileReader::getDefaultSampleRate()
{
    return 40000.0f;
}

int FileReader::getDefaultNumOutputs()
{
    return 16;
}

float FileReader::getDefaultBitVolts()
{
    return 0.0305f;
}

void FileReader::enabledState(bool t)
{

	isEnabled = t;

}


void FileReader::setFile(String fullpath)
{

    filePath = fullpath;

    const char* path = filePath.getCharPointer();

    if (input)
        fclose(input);

    input = fopen(path, "r");

    // Avoid a segfault if file isn't found
    if (!input)
    {
        std::cout << "Can't find data file "
                  << '"' << path << "\""
                  << std::endl;
        return;
    }

    fseek(input, 0, SEEK_END);
    lengthOfInputFile = ftell(input);
    rewind(input);

}


String FileReader::getFile()
{
    return filePath;
}

void FileReader::updateSettings()
{

}



void FileReader::process(AudioSampleBuffer& buffer, MidiBuffer& events, int& nSamples)
{

	uint8 data[8];
    memcpy(data, &timestamp, 8);

    // generate timestamp
    addEvent(events,    // MidiBuffer
             TIMESTAMP, // eventType
             0,         // sampleNum
             nodeId,    // eventID
             0,		 // eventChannel
             8,         // numBytes
             data   // data
            );

    // FIXME: needs to account for the fact that the ratio might not be an exact
    //        integer value
	int samplesNeeded = (int) float(buffer.getNumSamples()) * (getDefaultSampleRate()/44100.0f); 

	 if (ftell(input) >= lengthOfInputFile - samplesNeeded)
     {
        rewind(input);
     }

     size_t numRead = fread(readBuffer, 2, samplesNeeded*buffer.getNumChannels(), input);

    int chan = 0;
    int samp = 0;

    for (size_t n = 0; n < numRead; n++)
    {
        
        if (chan == buffer.getNumChannels())
        {
        	samp++;
            timestamp++;
            chan = 0;
        }

        *buffer.getSampleData(chan++, samp) = float(-readBuffer[n]) * getDefaultBitVolts(); // previously 0.035

    }

    nSamples = samplesNeeded;

}


void FileReader::setParameter(int parameterIndex, float newValue)
{

}



void FileReader::saveCustomParametersToXml(XmlElement* parentElement)
{

    XmlElement* childNode = parentElement->createNewChildElement("FILENAME");
    childNode->setAttribute("path", getFile());

}

void FileReader::loadCustomParametersFromXml()
{


    if (parametersAsXml != nullptr)
    {
        // use parametersAsXml to restore state 

        forEachXmlChildElement(*parametersAsXml, xmlNode)
        {
           if (xmlNode->hasTagName("FILENAME"))
            {
                String filepath = xmlNode->getStringAttribute("path");
               FileReaderEditor* fre = (FileReaderEditor*) getEditor();
                fre->setFile(filepath);

            }
        }   
    }

}