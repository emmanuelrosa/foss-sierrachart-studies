
/* DataFeedDelay.cpp

   These studies compare the current date/time with the date/time of the last bid/ask update, to compute the data feed delay in seconds. 

   These studies expect the chart's time zone to match your operating system's time zone.

   These studies automatically disable themselves during a chart replay.
   
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
SCDLLName("Data Feed Delay")

int inline DataFeedDelay(SCStudyInterfaceRef sc) {
    SCDateTime lastBidAskUpdateDateTime = sc.SymbolData->LastBidAskUpdateDateTime;

    lastBidAskUpdateDateTime.RoundDateTimeDownToSecond();
    lastBidAskUpdateDateTime = sc.ConvertDateTimeToChartTimeZone(lastBidAskUpdateDateTime, TIMEZONE_UTC);

    return (sc.CurrentSystemDateTime - lastBidAskUpdateDateTime).GetTimeInSeconds();
}

SCSFExport scsf_DataFeedDelayStudy(SCStudyInterfaceRef sc) {

    SCSubgraphRef delay = sc.Subgraph[0];

	if(sc.SetDefaults) {
		sc.GraphName = "Data Feed Delay Study";
        sc.StudyDescription = "Compares the current date/time with the date/time of the last bid/ask update, to compute the data feed delay in seconds. This study expects the chart's time zone to match your operating system's time zone.";
        sc.GraphRegion = 0;
		sc.AutoLoop = 1;
        sc.UpdateAlways = true;

        delay.Name = "Data Feed Delay (in seconds)";
        delay.DrawStyle = DRAWSTYLE_IGNORE;
        delay.PrimaryColor = COLOR_GREEN;

		return;
	}

    if(sc.IsFullRecalculation) return;
    if(sc.IsReplayRunning()) return;

    delay[sc.Index] = DataFeedDelay(sc);
}

SCSFExport scsf_DataFeedDelayAlertStudy(SCStudyInterfaceRef sc) {

    SCSubgraphRef delay = sc.Subgraph[0];

    SCInputRef delayThreshold = sc.Input[0];
    SCInputRef alertNumber = sc.Input[1];
    SCInputRef snoozeLength = sc.Input[2];

    int &allowAlert = sc.GetPersistentInt(0);
    int &snoozeMenuId = sc.GetPersistentInt(1);
    int &testMenuId = sc.GetPersistentInt(2);
    int &isSnoozed = sc.GetPersistentInt(3);
    SCDateTime &snoozeEndDateTime = sc.GetPersistentSCDateTime(0);

	if(sc.SetDefaults) {
		sc.GraphName = "Data Feed Delay Alert Study";
        sc.StudyDescription = "Compares the current date/time with the date/time of the last bid/ask update, to compute the data feed delay in seconds. If the delay exceeds the given threshold then the study issues an alert. The alert will continue to trigger until the data feed delay drops below the threshold. The alert can be snoozed via the chart's context (right-click) menu. Note: This study expects the chart's time zone to match your operating system's time zone.";
        sc.GraphRegion = 0;
		sc.AutoLoop = 1;
        sc.UpdateAlways = true;
		
        delay.Name = "Data Feed Delay (in seconds)";
        delay.DrawStyle = DRAWSTYLE_IGNORE;
        delay.PrimaryColor = COLOR_GREEN;

        alertNumber.Name = "Alert number";
        alertNumber.SetIntLimits(0, 150);
        alertNumber.SetInt(0);

        delayThreshold.Name = "Data feed delay threshold (in seconds)";
        delayThreshold.SetIntLimits(1, 1800); 
        delayThreshold.SetInt(10);
        
        snoozeLength.Name = "Snooze length (in seconds)";
        snoozeLength.SetIntLimits(5, 600);
        snoozeLength.SetInt(120);

		return;
	}

    if(sc.LastCallToFunction) {
        if(snoozeMenuId >= 0) sc.RemoveACSChartShortcutMenuItem(sc.ChartNumber, snoozeMenuId);
        return;
    }

    if(sc.Index == 0) {
        SCString snoozeMenuText, testMenuText;

        allowAlert = true;
        isSnoozed = false;
        snoozeMenuText.Format("Snooze data feed delay alert (study ID %i)", sc.StudyGraphInstanceID);
        testMenuText.Format("Test data feed delay alert (study ID %i)", sc.StudyGraphInstanceID);

        if(snoozeMenuId >= 0) sc.RemoveACSChartShortcutMenuItem(sc.ChartNumber, snoozeMenuId);
        if(testMenuId >= 0) sc.RemoveACSChartShortcutMenuItem(sc.ChartNumber, testMenuId);
        if(!sc.IsReplayRunning()) {
            snoozeMenuId = sc.AddACSChartShortcutMenuItem(sc.ChartNumber, snoozeMenuText);
            testMenuId = sc.AddACSChartShortcutMenuItem(sc.ChartNumber, testMenuText);

            if(snoozeMenuId < 0) sc.AddMessageToLog("ERROR: Unable to add data feed delay snooze menu.", 1);
            if(testMenuId < 0) sc.AddMessageToLog("ERROR: Unable to add data feed delay test menu.", 1);
        }
    }

    if(sc.IsFullRecalculation) return;
    if(sc.IsReplayRunning()) return;

    delay[sc.Index] = DataFeedDelay(sc);
    
    if(isSnoozed) {
        if(sc.CurrentSystemDateTime > snoozeEndDateTime) isSnoozed = false;
    }

    if(isSnoozed) return;

    bool triggerAlertTest = sc.MenuEventID == testMenuId;

    if(delay[sc.Index] > delayThreshold.GetInt()) {
        if(allowAlert) {
            SCString msg;

            msg.Format("The data feed is delayed by %f seconds, exceeding the threshold of %d seconds.", delay[sc.Index], delayThreshold.GetInt());
            sc.SetAlert(alertNumber.GetInt(), msg);
            allowAlert = false;
        } else {
            allowAlert = true;
        }

        if(sc.MenuEventID == snoozeMenuId) {
            SCString msg;

            SCDateTime snoozeDelay = sc.CurrentSystemDateTime;
            snoozeDelay += SCDateTime::SECONDS(snoozeLength.GetInt());
            snoozeEndDateTime = snoozeDelay;
            isSnoozed = true;
            msg.Format("Alert has been snoozed for %d seconds.", snoozeLength.GetInt());
            sc.AddAlertLine(msg, 0);
        }
    } else {
        allowAlert = true;

        if(triggerAlertTest) sc.SetAlert(alertNumber.GetInt(), "Testing data feed delay alert.");
    }
}
