/* BarCountPerDuration.cpp

   This study counts the number of bars within the given duration.
   The duration can be provided in hours, minutes, or seconds.

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
SCDLLName("Bar Count per Duration")

const SCString DURATION_UNIT_OPTIONS = "Hours;Minutes;Seconds";
const unsigned int MAX_DURATION = INT_MAX;

enum DurationUnitEnum {
    DURATION_HOURS
    , DURATION_MINUTES
    , DURATION_SECONDS
};

enum InputIndexEnum {
    DURATION_UNIT_INPUT
    , DURATION_VALUE_INPUT
};

enum PersistentVariableIndexEnum {
    END_DATE_TIME_VARIABLE
    , START_TIME_BAR_INDEX_VARIABLE
};

SCDateTime GetDuration(DurationUnitEnum durationUnit, unsigned int duration) {
    if(durationUnit == DURATION_HOURS) return SCDateTime::HOURS(duration);
    if(durationUnit == DURATION_MINUTES) return SCDateTime::MINUTES(duration);

    return SCDateTime::SECONDS(duration);
}

SCSFExport scsf_BarCountPerDuration(SCStudyInterfaceRef sc) {
    SCSubgraphRef barCountSubgraph = sc.Subgraph[0];
    SCInputRef durationUnitInput = sc.Input[DURATION_UNIT_INPUT];
    SCInputRef durationValueInput = sc.Input[DURATION_VALUE_INPUT];
    SCDateTime& endDateTime = sc.GetPersistentSCDateTime(END_DATE_TIME_VARIABLE);
    int& startTimeBarIndex = sc.GetPersistentInt(START_TIME_BAR_INDEX_VARIABLE);

	if(sc.SetDefaults) {
		sc.GraphName = "Bar Count per Duration";
        sc.StudyDescription = "This study counts the number of bars within the given duration. The duration can be provided in hours, minutes, or seconds.";
		sc.AutoLoop = 1;
        sc.GraphRegion = 1;
        sc.MaintainAdditionalChartDataArrays = true; // Required for sc.BaseDataEndDateTime
		
		barCountSubgraph.Name = "Bar count";
		barCountSubgraph.DrawStyle = DRAWSTYLE_LINE;
		barCountSubgraph.PrimaryColor = RGB (0, 255, 0);
		
		durationUnitInput.Name = "Duration unit";
		durationUnitInput.SetCustomInputStrings(DURATION_UNIT_OPTIONS);

        durationValueInput.Name = "Duration";
        durationValueInput.SetIntLimits(1, MAX_DURATION);
        durationValueInput.SetInt(1);
		
		return;
	}

    /* To understand how this study works, imagine that there's a string/cord which corresponds to the duration.
     * When the first bar is drawn on the chart, this study places both ends of the "string" at that first bar.
     * As bars continue to accumulate, one end of the string is left at that first bar, and the other end moves
     * along with the additional bars.
     * When the "string" is fully stretched, that means the bars within occured within the desired duration.
     * From this point forward, as bars accumulate the "string" pulls along with the latest bar.
     * As this happens, bars on the tail-end of the "string" drop off, and are no longer counted.
     */

    if(sc.IsFullRecalculation && sc.Index == 0) {
        SCDateTime barStartDateTime = sc.BaseDateTimeIn[sc.Index];
        endDateTime = barStartDateTime + GetDuration(static_cast<DurationUnitEnum>(durationUnitInput.GetIndex()), durationValueInput.GetInt());
        startTimeBarIndex = 0;

        return;
    }

    SCDateTime barEndDateTime = sc.BaseDataEndDateTime[sc.Index];

    if(barEndDateTime > endDateTime) {
        SCDateTime startDateTime = barEndDateTime - GetDuration(static_cast<DurationUnitEnum>(durationUnitInput.GetIndex()), durationValueInput.GetInt());
        endDateTime = barEndDateTime;

        if(sc.BaseDataEndDateTime[startTimeBarIndex] < startDateTime) {
            for(int i = startTimeBarIndex; i < sc.Index; i++) {
                if(sc.BaseDataEndDateTime[i] < startDateTime) { 
                    startTimeBarIndex++;
                } else {
                    // If the bar falls within the duration window, then there's no reason to keep looking.
                    break;
                }
            }
        }
    }

    barCountSubgraph[sc.Index] = sc.Index - startTimeBarIndex + 1;
}
