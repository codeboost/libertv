//-------------------------------------------------------------------------------------------------
// <copyright file="timeutil.cpp" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//
//    You must not remove this notice, or any other, from this software.
// </copyright>
//
// <summary>
//    Time helper functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "timeutil.h"
#include "strutil.h"

const LPCWSTR DAY_OF_WEEK[] = { L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat" };
const LPCWSTR MONTH_OF_YEAR[] = { L"None", L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun", L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec" };

const LPCWSTR DAY_OF_WEEK1[] = { L"Sunday", L"Monday", L"Tuesday", L"Wednesday", L"Thursday", L"Friday", L"Saturday" };
const LPCWSTR MONTH_OF_YEAR1[] = { L"None", L"January", L"February", L"March", L"April", L"May", L"June", L"July", L"August", L"September", L"October", L"November", L"December" };

enum TIME_PARSER { TimeNumber, TimeUnknown, DayOfWeek, DayOfMonth, MonthOfYear, Year, Hours, Minutes, Seconds, TimeZone};

// prototypes
static HRESULT DayFromString(
							 __in LPCWSTR wzDay,
							 __out WORD* pwDayOfWeek
							 );
static HRESULT MonthFromString(
							   __in LPCWSTR wzMonth,
							   __out WORD* pwMonthOfYear
							   );


BOOL IsNumber(LPCWSTR wzStart)
{
	LPCWSTR pPtr = wzStart; 
	while (pPtr && *pPtr)
	{
		if (!iswdigit(*pPtr))
			return FALSE; 
		pPtr++; 
	}
	return TRUE; 
}

//Returns DayOfWeek, Month, Year
TIME_PARSER GetTimeElement(LPCWSTR wzStart)
{
	if (IsNumber(wzStart))
	{
		ULONG number = wcstoul(wzStart, NULL, 10); 
		if (number > 1900)
			return Year; 
		return TimeNumber; 
	}

	WORD wParam; 

	HRESULT hr = DayFromString(wzStart, &wParam);
	if (SUCCEEDED(hr))
		return DayOfWeek; 

	hr = MonthFromString(wzStart, &wParam); 

	if (SUCCEEDED(hr))
		return MonthOfYear;

	return TimeUnknown; 

}

/********************************************************************
TimeFromString - converts string to FILETIME

*******************************************************************/
extern "C" HRESULT DAPI TimeFromString(
									   __in LPCWSTR wzTime,
									   __out FILETIME* pFileTime
									   )
{
	Assert(wzTime && pFileTime);

	HRESULT hr = S_OK;
	LPWSTR pwzTime = NULL;

	SYSTEMTIME sysTime = { 0 };
	TIME_PARSER timeParser = DayOfWeek;

	LPWSTR pwzStart = NULL;
	LPWSTR pwzEnd = NULL;

	hr = StrAllocString(&pwzTime, wzTime, 0);
	ExitOnFailure(hr, "Failed to copy time.");

	pwzStart = pwzEnd = pwzTime;
	while (pwzEnd && *pwzEnd)
	{
		if (L',' == *pwzEnd || L' ' == *pwzEnd || L':' == *pwzEnd)
		{
			*pwzEnd = L'\0'; // null terminate
			pwzEnd++;

			while (L' ' == *pwzEnd)
			{
				pwzEnd++; // and skip past the blank space
			}

			if (timeParser >= Hours)
				timeParser = (TIME_PARSER)((int)timeParser + 1);
			else
			{
				//expected time format:
				//"Sun, 01 Apr 2007 13:00:00 +0100"
				//Try to determine the time element
				TIME_PARSER pTimeElement = GetTimeElement(pwzStart); 
				if (pTimeElement != TimeUnknown && pTimeElement != TimeNumber)
					timeParser = pTimeElement; 
				else
				{
					//Work around for: "Thurs, May 17 2007 06:00:00 EST"
					if (timeParser == MonthOfYear && pTimeElement == TimeNumber)
						timeParser = DayOfMonth; 
					else
						timeParser = (TIME_PARSER)((int)timeParser + 1);
				}
			}

			switch (timeParser)
			{
			case DayOfWeek:
				hr = DayFromString(pwzStart, &sysTime.wDayOfWeek);
				ExitOnFailure1(hr, "Failed to convert string to day: %S", pwzStart);
				break;

			case DayOfMonth:
				sysTime.wDay = (WORD)wcstoul(pwzStart, NULL, 10);
				break;

			case MonthOfYear:
				hr = MonthFromString(pwzStart, &sysTime.wMonth);
				ExitOnFailure1(hr, "Failed to convert to month: %S", pwzStart);
				break;

			case Year:
				sysTime.wYear = (WORD)wcstoul(pwzStart, NULL, 10);
				break;

			case Hours:
				sysTime.wHour = (WORD)wcstoul(pwzStart, NULL, 10);
				if (sysTime.wHour == 24) sysTime.wHour = 0; 
				break;

			case Minutes:
				sysTime.wMinute = (WORD)wcstoul(pwzStart, NULL, 10);
				if (sysTime.wMinute >= 60) sysTime.wMinute = 0; 
				break;

			case Seconds:
				sysTime.wSecond = (WORD)wcstoul(pwzStart, NULL, 10);
				if (sysTime.wSecond >= 60) sysTime.wSecond = 0; 
				break;

			case TimeZone:
				// TODO: do something with this in the future, but this should only hit outside of the while loop.
				break;

			default:
				break;
			}

			pwzStart = pwzEnd;
		}

		++pwzEnd;
	}


	if (!::SystemTimeToFileTime(&sysTime, pFileTime))
	{
		ExitWithLastError(hr, "Failed to convert system time to file time.");
	}

LExit:
	ReleaseStr(pwzTime);
	return hr;
}


/********************************************************************
DayFromString - converts string to day

*******************************************************************/
static HRESULT DayFromString(
							 __in LPCWSTR wzDay,
							 __out WORD* pwDayOfWeek
							 )
{
	HRESULT hr = E_INVALIDARG; // assume we won't find a matching name

	for (WORD i = 0; i < countof(DAY_OF_WEEK); ++i)
	{
		if (0 == lstrcmpW(wzDay, DAY_OF_WEEK[i]))
		{
			*pwDayOfWeek = i;
			hr = S_OK;
			break;
		}
	}

	if (hr == S_OK)
		return hr; 

	//Try to find bogus spellings
	for (WORD i = 0; i < countof(DAY_OF_WEEK1); ++i)
	{
		if (NULL != wcsstr(DAY_OF_WEEK1[i], wzDay))
		{
			*pwDayOfWeek = i;
			hr = S_OK;
			break;
		}
	}
	return hr;
}


/********************************************************************
MonthFromString - converts string to month

*******************************************************************/
static HRESULT MonthFromString(
							   __in LPCWSTR wzMonth,
							   __out WORD* pwMonthOfYear
							   )
{
	HRESULT hr = E_INVALIDARG; // assume we won't find a matching name

	for (WORD i = 0; i < countof(MONTH_OF_YEAR); ++i)
	{
		if (0 == lstrcmpW(wzMonth, MONTH_OF_YEAR[i]))
		{
			*pwMonthOfYear = i;
			hr = S_OK;
			break;
		}
	}

	if (hr == S_OK)
		return hr; 

	//Some RSS comes with "Thurs" instead of "Thu" or "Thursday"
	for (WORD i = 0; i < countof(MONTH_OF_YEAR1); ++i)
	{
		if (0 != wcsstr(MONTH_OF_YEAR1[i], wzMonth))
		{
			*pwMonthOfYear = i;
			hr = S_OK;
			break;
		}
	}

	return hr;
}