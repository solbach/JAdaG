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

#include "jadag.h"

#define VERSION 0.5


JAdaG::~JAdaG(void)
{
    for(std::vector<gps*>::iterator iter = mGps.begin();
        iter != mGps.end(); iter++)
    {
        delete (*iter);
    }
}

JAdaG::JAdaG(std::string imgDir, std::string gpxDir)
{
    mImgDir = imgDir;
    mGpxDir = gpxDir;
    mAvDistance = 0;
}

JAdaG::JAdaG(std::string imgDir)
{
    mImgDir = imgDir;
}

void JAdaG::parseGPX(void)
{
    char* sChar, *stopstring;
    sChar = const_cast<char*>(mGpxDir.c_str());

    // Loop over all gpx-Files within the given Folder
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (sChar)) != NULL) {
      /* get all the files and directories within directory */
      while ((ent = readdir (dir)) != NULL) {


          if(jdg::checkGpxFileName(ent->d_name))
          {
                std::string fileName = mGpxDir + ent->d_name;
                std::cout << "\t READING: " << fileName << std::endl;
                // this open and parse the XML file:
                XMLNode xMainNode=XMLNode::openFileHelper(fileName.c_str(),
                                                                        "gpx");

                // Part 1: Iterate over all <trk> - elements and its childs
                // go to trk tag -> this where all the information is
                XMLNode trkNode = xMainNode.getChildNode("trk");

                // get number of trkseg tags to iterate over them
                int nTrkSeg = trkNode.nChildNode("trkseg");

                // iterate over all found trkseg tags
                for (int i=0; i<nTrkSeg; i++)
                {
                    // get number of trkpt to iterate over them
                    int nTrkpt = trkNode.getChildNode("trkseg",i).nChildNode("trkpt");

                    for (int j=0; j<nTrkpt; j++)
                    {
                        gps* actGpsOne = new gps();

                        actGpsOne->setlatitude( atof(trkNode.getChildNode("trkseg",i).
                                                 getChildNode("trkpt", j).
                                                 getAttribute("lat")) );

                        actGpsOne->setlongitude( atof(trkNode.getChildNode("trkseg",i).
                                                  getChildNode("trkpt", j).
                                                  getAttribute("lon")) );

                        actGpsOne->setaltitude( atof(trkNode.getChildNode("trkseg",i).
                                                 getChildNode("trkpt", j).
                                                 getChildNode("ele").getText()) );

                        std::string time( trkNode.getChildNode("trkseg",i).
                                     getChildNode("trkpt", j).
                                     getChildNode("time").getText() );

                        actGpsOne->settimestamp(jdg::dateString2ullong(time));

                        mGps.push_back(actGpsOne);
                    }
                }

                // Part 2: Iterate over all <wpt> - elements
                int nWpt = xMainNode.nChildNode("wpt");
                for(int j = 0; j < nWpt; j++)
                {
                    gps* actGpsTwo = new gps();
                    actGpsTwo->setlatitude( atof(xMainNode.getChildNode("wpt", j).
                                                 getAttribute("lat")) );

                    actGpsTwo->setlongitude( atof(xMainNode.getChildNode("wpt", j).
                                                 getAttribute("lon")) );

                    actGpsTwo->setaltitude( atof(xMainNode.getChildNode("wpt", j).
                                                 getChildNode("ele").getText()) );

                    std::string time( xMainNode.getChildNode("wpt", j).
                                 getChildNode("time").getText() );

                    actGpsTwo->settimestamp(jdg::dateString2ullong(time));

                    mGps.push_back(actGpsTwo);
                }
            }
      }
      std::cout << "\n" << "\t Total GPS Data found: " << mGps.size() << std::endl;
    }
}


void JAdaG::apply(void)
{
    char* sChar, *stopstring;
    sChar = const_cast<char*>(mImgDir.c_str());

    unsigned long long imgDate;
    int count = 1;

    gps* bestGPS = new gps(0.0, 0.0, 0.0, 99999999999999);

    Exiv2::ExifData exifData;

    std::cout << "\n" << std::endl;

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (sChar)) != NULL) {
      /* get all the files and directories within directory */
      while ((ent = readdir (dir)) != NULL) {
          if(jdg::checkImgFileName(ent->d_name))
          {
              std::cout << "\t EXECUTING " << count++ << ".Img: " << ent->d_name << std::endl;
              // load Image
              Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(mImgDir +
                                                                   ent->d_name);

              // read Exif Data
              image->readMetadata();
              exifData = image->exifData();

              // read Date/Time
              Exiv2::Exifdatum& tag = exifData["Exif.Photo.DateTimeOriginal"];
              std::string date = tag.toString();

              // Parse it to unsigned long long
              imgDate = jdg::dateString2ullong(date);

              // Reset bestGPS Timestamp before next loop
              bestGPS->settimestamp(99999999999999);

              // loop over all gps
              for(int i = 0; i < mGps.size(); i++)
              {
                  // get gps time
                  // calculate distance
                  // remember shortest distance of I
                  if(jdg::euclDistance(mGps[i]->timestamp(), imgDate)
                          < jdg::euclDistance(bestGPS->timestamp(), imgDate))
                  {
                      bestGPS = mGps[i];
                  }
              }
              // end loop

              std::cout << "\t \t GPS Found - " << "Distance: " <<
                       abs(imgDate - bestGPS->timestamp()) << std::endl;
              mAvDistance += abs(imgDate - bestGPS->timestamp());
              // write gps coord. from I into the act. exif
              jdg::writeGpsToExif(bestGPS, exifData);

              // Write Informationen to Image
              image->setExifData(exifData);
              image->writeMetadata();
          }
      }
      // end loop
      mAvDistance /= count;
      closedir (dir);

      std::cout << "Average Distance: " << mAvDistance << std::endl;

    } else {
      /* could not open directory */
      perror ("");
    }
}

void JAdaG::applyRemoval(void)
{
    char* sChar, *stopstring;
    sChar = const_cast<char*>(mImgDir.c_str());

    unsigned long long imgDate;
    int count = 1;

    Exiv2::ExifData exifData;

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (sChar)) != NULL) {
      /* get all the files and directories within directory */
      while ((ent = readdir (dir)) != NULL) {
          if(jdg::checkImgFileName(ent->d_name))
          {
              std::cout << "\t Removing GPS from " << count++ << ".Img: " << ent->d_name << std::endl;
              // load Image
              Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(mImgDir +
                                                                   ent->d_name);

              // read Exif Data
              image->readMetadata();
              exifData = image->exifData();

              // call removal function
              jdg::deleteGpsFromExif(exifData);

              // Write Informationen to Image
              image->setExifData(exifData);
              image->writeMetadata();
          }
      }

      // end loop
      closedir (dir);
    } else {
      /* could not open directory */
      perror ("");
    }
}



int main(int argc, char* const argv[])
try {

    std::cout << "\t JAdaG v" << VERSION << "\n\n" << std::endl;

    if (argc < 2) {
        std::cout << "Usage: <IMG-DIR> <GPX-DIR> \n";
        std::cout << "Usage: <IMG-DIR> to remove GPS Data \n";
        return 1;
    }

    if(argc == 2)
    {
        JAdaG* jadag = new JAdaG(argv[1]);

        // delete GPS from Images
        jadag->applyRemoval();

        // delete all objects from memory
        delete jadag;
        return 0;
    }
    else
    {
        JAdaG* jadag = new JAdaG(argv[1], argv[2]);

        // Parse the given GPX File
        jadag->parseGPX();

        // Loop over all Pictures and look up the closest GPS entry
        // and write it into exif
        jadag->apply();

        // delete all objects from memory
        delete jadag;
        return 0;
    }
}
catch (Exiv2::AnyError& e) {
    std::cout << "Caught Exiv2 exception '" << e.what() << "'\n";
    return -1;
}
