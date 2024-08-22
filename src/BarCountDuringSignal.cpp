/* BarCountDuringSignal.cpp

   This study counts the number of bars while the given subgraph value is non-zero.
   The subgraph produced by this study contains the count.
   
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
SCDLLName("Bar Count During Signal Study")

SCSFExport scsf_TemplateFunction(SCStudyInterfaceRef sc) {
    SCSubgraphRef count = sc.Subgraph[0];
    SCInputRef signalInput = sc.Input[0];
    SCFloatArray signal;

	if(sc.SetDefaults) {
		sc.GraphName = "Bar Count During Signal Study";
        sc.StudyDescription = "This study counts the number of bars while the given subgraph value is non-zero. The subgraph produced by this study contains the count.";
		sc.AutoLoop = 1;
        
        signalInput.Name = "Subgraph to use as the signal to count.";
        signalInput.SetChartStudySubgraphValues(1,1, 0);

		count.Name = "Count";
		count.DrawStyle = DRAWSTYLE_LINE;
		count.PrimaryColor = RGB (0, 255, 0);
        count.DrawZeros = 1;
		
		return;
	}

    sc.GetStudyArrayFromChartUsingID(signalInput.GetChartStudySubgraphValues(), signal);

    if(signal[sc.Index] != 0) {
        count[sc.Index] = count[sc.Index - 1] + 1;
    }
}
