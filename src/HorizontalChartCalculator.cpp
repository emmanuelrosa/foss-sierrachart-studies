/* HorizontalChartCalculator.cpp

   This study Displays a horizontal line at the given price, and the offset from that price to the current price.
   
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
SCDLLName("Horizontal Chart Calculator")

SCSFExport scsf_HorizontalChartCalculator(SCStudyInterfaceRef sc) {

    SCSubgraphRef line = sc.Subgraph[0];
    SCInputRef price = sc.Input[0];
    SCInputRef textColor = sc.Input[1];
    SCInputRef textBackgroundColor = sc.Input[2];
    SCInputRef fontSize = sc.Input[3];
    SCInputRef textAlignment = sc.Input[4];

    int &startIndex = sc.GetPersistentInt(0);
    int &endIndex = sc.GetPersistentInt(1);
    int &toolLineNumber = sc.GetPersistentInt(2);

    if(sc.SetDefaults) {
        sc.GraphName = "Horizontal Chart Calculator";
        sc.StudyDescription = "Displays a horizontal line at the given price, and the offset from that price to the current price";
        sc.AutoLoop = 1;
        sc.GraphRegion = 0;
        sc.UpdateAlways = 1;
        
        line.Name = "Price line";
        line.DrawStyle = DRAWSTYLE_LINE;
        
        price.Name = "Price";
        price.SetFloat(0);
        
        textColor.Name = "Text color";
        textColor.SetColor(COLOR_WHITE);
        
        textBackgroundColor.Name = "Text background color";
        textBackgroundColor.SetColor(COLOR_BLACK);
        
        fontSize.Name = "Font size";
        fontSize.SetInt(8);
        fontSize.SetIntLimits(8, 112);
        
        textAlignment.Name = "Text alignment";
        textAlignment.SetCustomInputStrings("Left;Right");
        textAlignment.SetCustomInputIndex(1);
		
        return;
    }

    if(sc.Index == 0) {
        startIndex = 0;
        endIndex = 0;
        toolLineNumber = 0;
    }

    if(sc.IsFullRecalculation) return;

    s_UseTool tool;
    const float last = sc.BaseDataIn[SC_LAST][sc.Index];
    const float priceDifference = last - price.GetFloat();
    const float tickDifference = priceDifference / sc.TickSize;
    const float currencyValue = tickDifference * sc.CurrencyValuePerTick;

    tool.Clear();
    tool.LineNumber = toolLineNumber != 0 ? toolLineNumber : -1;

    // Erase and redraw the horizontal line when the chart's visible area changes.
    if(startIndex != sc.IndexOfFirstVisibleBar
        || endIndex != sc.IndexOfLastVisibleBar) {
        const int alignment = textAlignment.GetIndex() == 1 ? DT_RIGHT : DT_LEFT;
        const int beginIndex = textAlignment.GetIndex() == 1 ? sc.IndexOfLastVisibleBar : sc.IndexOfFirstVisibleBar;

        for(int i = startIndex; i <= endIndex; i++) {
            line[i] = 0;
        }

        for(int i = sc.IndexOfFirstVisibleBar; i <= sc.IndexOfLastVisibleBar; i++) {
            line[i] = price.GetFloat();
        }

        tool.ChartNumber = sc.ChartNumber;
        tool.Region = sc.GraphRegion;
        tool.DrawingType = DRAWING_TEXT;
        tool.AddMethod = UTAM_ADD_OR_ADJUST;
        tool.BeginIndex = beginIndex;
        tool.BeginValue = price.GetFloat();
        tool.FontSize = fontSize.GetInt();
        tool.Color = textColor.GetColor();
        tool.FontBackColor = textBackgroundColor.GetColor();
        tool.TextAlignment = DT_BOTTOM | alignment;
        sc.UseTool(tool);

        toolLineNumber = tool.LineNumber;
        startIndex = sc.IndexOfFirstVisibleBar;
        endIndex = sc.IndexOfLastVisibleBar;
    }

    tool.Text.Format("DIT: %f CV: %f PD: %f", tickDifference, currencyValue, priceDifference);
    sc.UseTool(tool);
}
