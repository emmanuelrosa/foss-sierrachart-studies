/* ExportToCSV.cpp

   This study exports 11 subgraphs to a (comma-separated-value (CSV) file, for easy import into a spreadsheet application such as Microsoft Excel. The study will continue to append data to the CSV file as long as it's running. Note: The last bar's data is not exported.

   MIT License
   
   Copyright (c) 2025 Emmanuel Rosa
   
   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
   
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
   
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

#include "sierrachart.h"
SCDLLName("Export to CSV")
#include <random>

/* A pseudo higher-order function which iterates through the SCInputRef's which are used to specify the subgraphs to export.
 * Implemented as a macro because lamdas don't work in Sierra Chart studies.
 */
#define ForEachDataInput { \
    const unsigned int INPUT_INDEX_START = 2; \
    const unsigned int INPUT_INDEX_END = 12; \
    for(int i = INPUT_INDEX_START; i <= INPUT_INDEX_END; i++) { \
        SCInputRef input = sc.Input[i]; \
        unsigned index = i - INPUT_INDEX_START; \

#define EndForEach }}

SCSFExport scsf_ExportSubgraphsToCSV(SCStudyInterfaceRef sc) {
    SCSubgraphRef dummySubgraph = sc.Subgraph[0];
    SCInputRef outputFileInput = sc.Input[0];
    SCInputRef headerFormatInput = sc.Input[1];
    int &fileHandle = sc.GetPersistentInt(0);
    int &lastIndex = sc.GetPersistentInt(1);
    int &exportHeader = sc.GetPersistentInt(2);

	if(sc.SetDefaults) {
        sc.GraphName = "Export 11 Subgraphs to CSV";
        sc.StudyDescription = "This study exports 11 subgraphs to a (comma-separated-value (CSV) file, for easy import into a spreadsheet application such as Microsoft Excel. The study will continue to append data to the CSV file as long as it's running. Note: The last bar's data is not exported.";
        sc.AutoLoop = 1;
        sc.UpdateAlways = 1;

        std::random_device r;
        std::default_random_engine e1(r());
        std::uniform_int_distribution<int> uniform_dist(1, INT_MAX);
		outputFileInput.Name = "Output file";
		outputFileInput.SetPathAndFileName(sc.DataFilesFolder().AppendFormat("\%s-%d.csv", "subgraph-export", uniform_dist(e1)));

        headerFormatInput.Name = "Header format";
        headerFormatInput.SetCustomInputStrings("Chart, study, & subgraph names;Study & subgraph names;Subgraph name"); 
        headerFormatInput.SetCustomInputIndex(0);
		
        ForEachDataInput
            input.Name.Format("Subgraph to export #%d", index + 1);
            input.SetChartStudySubgraphValues(sc.ChartNumber, 0, index);
        EndForEach

		return;
	}

    /* When the study recalculates, close the file if it's already open, and reopen it.
     * This causes the file to be re-written to avoid duplicating data.
     */
    if(sc.Index == 0) {
        lastIndex = -1;
        exportHeader = true;

        if(fileHandle) {
            if(!sc.CloseFile(fileHandle)) sc.AddMessageToLog(sc.GetLastFileErrorMessage(fileHandle), 1);
            fileHandle = 0;
        }

        if(fileHandle == 0) {
            int handle = 0;

            if(!sc.OpenFile(outputFileInput.GetPathAndFileName(), n_ACSIL::FILE_MODE_OPEN_TO_REWRITE_FROM_START, handle)) {
                sc.AddMessageToLog("ERROR: Unable to open the file.", 1);
            }

            fileHandle = handle;
        }
    }

    /* Close the file when the study is removed from the chart,
     * or when the study DLL is unloaded.
     */
    if(sc.LastCallToFunction) {
        if(fileHandle) {
            if(!sc.CloseFile(fileHandle)) sc.AddMessageToLog(sc.GetLastFileErrorMessage(fileHandle), 1);
            fileHandle = 0;
        }
    }

    /* Wait until the study recalculation is finished.
     * This is to perform the first export as a batch,
     * which is more efficient.
     */
    if(sc.IsFullRecalculation) return;

    /* When a new bar opens, export the prior bar's subgraph data.
     * This also handles the first batch export.
     */
    if(fileHandle != 0 && sc.Index > lastIndex) {
        SCString headerStringBuffer;
        int headerFormat = headerFormatInput.GetIndex();

        ForEachDataInput
            sc.GetStudyArrayFromChartUsingID(input.GetChartStudySubgraphValues(), dummySubgraph.Arrays[index]);
        EndForEach

        // Construct the header row of the CSV file, and write it.
        if(exportHeader) {
            unsigned int bytesWritten = 0;
            headerStringBuffer = "\"Date Time\"";

            ForEachDataInput
                SCString subgraphName;
                s_ChartStudySubgraphValues sv = input.GetChartStudySubgraphValues();

                subgraphName.Format("SG%d", sv.SubgraphIndex + 1);
                sc.GetStudySubgraphNameFromChart(sc.ChartNumber, sv.StudyID, sv.SubgraphIndex, subgraphName);

                if(headerFormat == 0) {
                    headerStringBuffer.AppendFormat(",\"%s %s %s\"", sc.GetChartName(sv.ChartNumber).GetChars(), sc.GetStudyNameFromChart(sv.ChartNumber, sv.StudyID).GetChars(), subgraphName.GetChars());
                } else if(headerFormat == 1) {
                    headerStringBuffer.AppendFormat(",\"%s %s\"", sc.GetStudyNameFromChart(sv.ChartNumber, sv.StudyID).GetChars(), subgraphName.GetChars());
                } else {
                    headerStringBuffer.AppendFormat(",\"%s\"", subgraphName.GetChars());
                }
            EndForEach
            headerStringBuffer.Append("\r\n");

            const char* buffer = headerStringBuffer.GetChars();
            if(!sc.WriteFile(fileHandle, buffer, headerStringBuffer.GetLength(), &bytesWritten)) sc.AddMessageToLog(sc.GetLastFileErrorMessage(fileHandle), 1);
            exportHeader = false;
        }

        for(int row = max(0, lastIndex); row < sc.Index; row++) {
            unsigned int bytesWritten = 0;
            SCString dataStringBuffer;

            dataStringBuffer.Format("\"%s\"", sc.DateTimeToString(sc.BaseDateTimeIn[row], FLAG_DT_COMPLETE_DATETIME).GetChars()); 

            ForEachDataInput
                dataStringBuffer.AppendFormat(",\"%f\"", dummySubgraph.Arrays[index][row]);
            EndForEach
            dataStringBuffer.Append("\r\n");

            const char* buffer = dataStringBuffer.GetChars();
            if(!sc.WriteFile(fileHandle, buffer, dataStringBuffer.GetLength(), &bytesWritten)) sc.AddMessageToLog(sc.GetLastFileErrorMessage(fileHandle), 1);
        }

        lastIndex = sc.Index;
    }
}
