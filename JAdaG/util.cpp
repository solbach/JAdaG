/***********************************************************************
*
* JAdaG
*
* This cmd-line Tool attaches GPS-Data to your images using timestamps
* Copyright (C) 2013 Markus Solbach <solbach.m@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include "util.h"

namespace jdg
{
    unsigned long long dateString2ullong(const std::string& timeVal)
    {
        std::string time = timeVal;
        // remove all unnecessary characters
        time.erase(4,1);
        time.erase(6,1);
        time.erase(8,1);
        time.erase(10,1);
        time.erase(12,1);

        // check whether the string is that long first
        if(time.size() > 14)
        {
        time.erase(14, time.size() - 14);
        }

        // convert String to char
        char* sChar, *stopstring;
        sChar = const_cast<char*>(time.c_str());

        // convert to ulong long and return
        return strtoul(sChar, &stopstring, BASE);
    }

    bool checkImgFileName (const std::string& file)
    {
        return (file[0] == '.' ? false : true);
    }

    bool checkGpxFileName (const std::string& file)
    {
        if(file.size() < 4)
        {
            return false;
        }
        else
        {
            std::string tempName = file.substr(file.size()-4, file.size()-1);
            return (((tempName == ".gpx") || (tempName == ".GPX")) ? true : false);
        }
    }

    unsigned long long euclDistance(const unsigned long long& val1,
                                    const unsigned long long& val2 )
    {
        return abs(val1 - val2);
    }

     void writeGpsToExif(gps *bestGPS, Exiv2::ExifData& exifData)
     {
         // arc minute
         char tmp[50];

         // Altitude
         snprintf(tmp, 50, "%d/10", (int)(bestGPS->altitude() * 10));
         exifData["Exif.GPSInfo.GPSAltitude"] = std::string(tmp);
         exifData["Exif.GPSInfo.GPSAltitudeRef"] = (bestGPS->altitude() < 0) ? "1" :"0";

         double absVal, remai;
         int deg, min, sec;

         // Latitude
         absVal = fabs(bestGPS->latitude());
         deg = (int)absVal;

         remai = absVal - deg;
         min = floor(remai * 60);

         remai = (remai * 60) - min;
         sec = (remai * 60 * 100000);

         snprintf(tmp, 50, "%d/1 %d/1 %d/100000", deg, min, sec);

         exifData["Exif.GPSInfo.GPSLatitude"] = std::string(tmp);
         exifData["Exif.GPSInfo.GPSLatitudeRef"] = (bestGPS->latitude() < 0) ? "S" :"N";

         // Longitude
         absVal = fabs(bestGPS->longitude());
         deg = (int)absVal;

         remai = absVal - deg;
         min = floor(remai * 60);

         remai = (remai * 60) - min;
         sec = (remai * 60 * 100000);

         snprintf(tmp, 50, "%d/1 %d/1 %d/100000", deg, min, sec);

         exifData["Exif.GPSInfo.GPSLongitude"] = std::string(tmp);
         exifData["Exif.GPSInfo.GPSLongitudeRef"] = (bestGPS->longitude() <0) ? "W":"E";
     }

     void deleteGpsFromExif(Exiv2::ExifData& exifData)
     {

         Exiv2::ExifKey key("Exif.GPSInfo.GPSAltitude");;
         Exiv2::ExifData::iterator pos;

         // find altitude
         key = Exiv2::ExifKey("Exif.GPSInfo.GPSAltitude");
         pos = exifData.findKey(key);

         // delete altitude
         if (pos == exifData.end())
         {
             std::cout << "\t \t not found: " << key << std::endl;
         }
         else
         {
             exifData.erase(pos);
         }

         // find altituderef
         key = Exiv2::ExifKey("Exif.GPSInfo.GPSAltitudeRef");
         pos = exifData.findKey(key);

         // delete AltitudeRef
         if (pos == exifData.end())
         {
             std::cout << "\t \t not found: " << key << std::endl;
         }
         else
         {
             exifData.erase(pos);
         }

         // find GPSLatitude
         key = Exiv2::ExifKey("Exif.GPSInfo.GPSLatitude");
         pos = exifData.findKey(key);

         // delete GPSLatitude
         if (pos == exifData.end())
         {
             std::cout << "\t \t not found: " << key << std::endl;
         }
         else
         {
             exifData.erase(pos);
         }

         // find GPSLatitudeRef
         key = Exiv2::ExifKey("Exif.GPSInfo.GPSLatitudeRef");
         pos = exifData.findKey(key);

         // delete GPSLatitudeRef
         if (pos == exifData.end())
         {
             std::cout << "\t \t not found: " << key << std::endl;
         }
         else
         {
             exifData.erase(pos);
         }

         // find GPSLongitude
         key = Exiv2::ExifKey("Exif.GPSInfo.GPSLongitude");
         pos = exifData.findKey(key);

         // delete GPSLongitude
         if (pos == exifData.end())
         {
             std::cout << "\t \t not found: " << key << std::endl;
         }
         else
         {
             exifData.erase(pos);
         }

         // find GPSLongitudeRef
         key = Exiv2::ExifKey("Exif.GPSInfo.GPSLongitudeRef");
         pos = exifData.findKey(key);

         // delete GPSLongitudeRef
         if (pos == exifData.end())
         {
             std::cout << "\t \t not found: " << key << std::endl;
         }
         else
         {
             exifData.erase(pos);
         }
     }
}
