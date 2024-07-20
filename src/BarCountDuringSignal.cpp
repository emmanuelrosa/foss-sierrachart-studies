/* BarCountDuringSignal.cpp

   This study counts the number of bars while the given subgraph value is non-zero.
   The subgraph produced by this study contains the count.
   
   Copyright (C) 2024 Emmanuel Rosa

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
