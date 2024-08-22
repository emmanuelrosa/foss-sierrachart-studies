/* HighestBarCountDuringSignal.cpp

   This study counts the number of times the input signal contains a non-zero value, 
   during the given n number of bars. A percentage is also available as a second
   subgraph.
   
   MIT License
   
   Copyright (c) 2024 Emmanuel Rosa
   
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
SCDLLName("Signal Count per Number of Bars")

SCSFExport scsf_SignalCountPerNumberOfBars(SCStudyInterfaceRef sc) {
    SCSubgraphRef count = sc.Subgraph[0];
    SCSubgraphRef percentage = sc.Subgraph[1];
    SCInputRef signal = sc.Input[0];
    SCInputRef length = sc.Input[1];

    SCFloatArray signalData;

    int& lastIndex = sc.GetPersistentInt(0);
    int& lastCount = sc.GetPersistentInt(1);

	if(sc.SetDefaults) {
		sc.GraphName = "Signal Count per Number of Bars";
        sc.StudyDescription = "This study counts the number of times the input signal contains a non-zero value, during the given n number of bars. A percentage is also available as a second subgraph.";
		sc.AutoLoop = 1;
		
		count.Name = "Count";
		count.DrawStyle = DRAWSTYLE_LINE;
		count.PrimaryColor = RGB (0, 255, 0);

		percentage.Name = "Percentage";
		percentage.DrawStyle = DRAWSTYLE_IGNORE;
		
		signal.Name = "Input signal";
		signal.SetChartStudySubgraphValues(1, 1, 0);

		length.Name = "Length";
		length.SetInt(10);
		
		return;
	}
	
    if(sc.Index == 0) {
        lastIndex = -1;
        lastCount = 0;
    }

    if(sc.Index + 1 < length.GetInt()) return;

    sc.GetStudyArrayFromChartUsingID(signal.GetChartStudySubgraphValues(), signalData);

    if(sc.Index != lastIndex) {
        lastCount = 0;

        for(int i = sc.Index - length.GetInt() + 1; i < sc.Index; i++) { 
            if(signalData[i] != 0) lastCount++;
        }

        lastIndex = sc.Index;
    }

    const int currentCount = signalData[sc.Index] == 0 ? lastCount : lastCount + 1;
    count[sc.Index] = currentCount;
    percentage[sc.Index] = (float)currentCount / (float)length.GetInt();
}
