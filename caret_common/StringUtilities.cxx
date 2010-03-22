/*LICENSE_START*/
/*
 *  Copyright 1995-2002 Washington University School of Medicine
 *
 *  http://brainmap.wustl.edu
 *
 *  This file is part of CARET.
 *
 *  CARET is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  CARET is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CARET; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/*LICENSE_END*/

#include <algorithm>
#include <iostream>
#include <sstream>

#include <ctype.h>

#include <QString>
#include <QStringList>

#include "DebugControl.h"

#define __STRING_UTILITIES_MAIN_H__
#include "StringUtilities.h"
#undef __STRING_UTILITIES_MAIN_H__

/**
 * sort elements case insensitive.
 */
static bool
lessThanCaseInsensitive(const QString& s1, const QString& s2) 
{
   return (QString::compare(s1, s2, Qt::CaseInsensitive) < 0);
}
static bool
compareCaseInsensitive(const QString& s1, const QString& s2) 
{
   return (QString::compare(s1, s2, Qt::CaseInsensitive) == 0);
}
void 
StringUtilities::sortCaseInsensitive(std::vector<QString>& elements,
                                     const bool reverseOrderFlag,
                                     const bool removeDuplicatesFlag)
{
   std::sort(elements.begin(), 
             elements.end(), 
             lessThanCaseInsensitive);
   
   if (removeDuplicatesFlag) {
      std::vector<QString>::iterator it = std::unique(elements.begin(), 
                                                      elements.end(),
                                                      compareCaseInsensitive);
      if (it != elements.end()) {
         elements.resize(it - elements.begin());
      }
   }

   if (reverseOrderFlag) {
      std::reverse(elements.begin(), elements.end());
   }   
}

/**
 * replace part of a string
 */
QString
StringUtilities::replace(const QString& s, const QString& findThis, 
              const QString& replaceWith)
{
   QString str(s);
   const int findThisLength = findThis.length();
   int pos = str.indexOf(findThis);
   while (pos != -1) {
      str.replace(pos, findThisLength, replaceWith);
      pos = str.indexOf(findThis);
   }
   return str;
}

/**
 * replace part of a string
 */
QString
StringUtilities::replace(const QString& s, const char findThis, 
              const char replaceWith)
{
   QString str(s);
   for (int i = 0; i < str.length(); i++) {
      if (str[i] == findThis) {
         str[i] = replaceWith;
      }
   }
   return str;
}

/**
 * split "s" into tokens breaking at whitespace but maintaining string in double quotes.
 */
void 
StringUtilities::tokenStringsWithQuotes(const QString& s,
                                        QStringList& stringsOut)
{
   stringsOut.clear();
   
   bool done = false;
   int pos = 0;
   while (done == false) {
      const int quoteIndex = s.indexOf('\"', pos);
      if (quoteIndex >= 0) {
         //
         // Get stuff before double quote
         //
         const int startIndex = pos;
         const int endIndex = quoteIndex - 1;
         if (endIndex > startIndex) {
            const QString str = s.mid(startIndex, endIndex - startIndex + 1);
            stringsOut << str.split(QRegExp("\\s+"), QString::SkipEmptyParts);
         }
         
         //
         // Find next double quote
         //
         const int nextQuoteIndex = s.indexOf('\"', quoteIndex + 1);
         if (nextQuoteIndex >= 0) {
            //
            // Include doubled quotes
            //
            const int startIndex = quoteIndex;
            const int endIndex = nextQuoteIndex;
            if (endIndex > startIndex) {
               const QString str = s.mid(startIndex, endIndex - startIndex + 1);
               stringsOut << str;  // DO NOT SPLIT ON WHITESPACE
            }
            //
            // Next character to search
            //
            pos = nextQuoteIndex + 1;
         }
         else {
            //
            // Did not find second double quote so gobble rest of string
            // and add double quote
            //
            const int startIndex = quoteIndex;
            QString str = s.mid(startIndex);
            if (str.isEmpty() == false) {
               str += '\"';
               stringsOut << str;
            }
            done = true;
         }
         
      }
      else {
         //
         // Get rest of string split at white space
         //
         const QString str = s.mid(pos);
         if (str.isEmpty() == false) {
            stringsOut << str.split(QRegExp("\\s+"), QString::SkipEmptyParts);
         }
         done = true;
      }
   }

   if (DebugControl::getDebugOn()) {
      std::cout << "STRING IN: " << s.toAscii().constData() << std::endl;
      std::cout << "STRING SPLIT: " << std::endl;
      for (int i = 0; i < stringsOut.count(); i++) {
         std::cout << "   [" << stringsOut.at(i).toAscii().constData() << "]" << std::endl;
      }
   }
}
                                         
/**
 * like strtok() function.
 * Split string "s" at any of the characters in "separators" and place each
 * piece in float tokens.
 */
void 
StringUtilities::token(const QString& s, 
            const QString& separators,
            std::vector<float>& tokenFloats)
{
   std::vector<QString> tokens;
   token(s, separators, tokens);
   
   tokenFloats.clear();
   for (unsigned int i = 0; i < tokens.size(); i++) {
      tokenFloats.push_back(toFloat(tokens[i]));
   }
}

/**
 * like strtok() function.
 * Split string "s" at any of the characters in "separators" and place each
 * piece in double tokens.
 */
void 
StringUtilities::token(const QString& s, 
            const QString& separators,
            std::vector<double>& tokenDoubles)
{
   std::vector<QString> tokens;
   token(s, separators, tokens);
   
   tokenDoubles.clear();
   for (unsigned int i = 0; i < tokens.size(); i++) {
      tokenDoubles.push_back(toDouble(tokens[i]));
   }
}

/**
 * like strtok() function.
 * Split string "s" at any of the characters in "separators" and place each
 * piece in int tokens.
 */
void 
StringUtilities::token(const QString& s, 
            const QString& separators,
            std::vector<int>& tokenInts)
{
   std::vector<QString> tokens;
   token(s, separators, tokens);
   
   tokenInts.clear();
   for (unsigned int i = 0; i < tokens.size(); i++) {
      tokenInts.push_back(toInt(tokens[i]));
   }
}

/**
 * split "s" into "bool tokens" splitting with any characters in "separators".
 */
void
StringUtilities::token(const QString& s,
                       const QString& separators,
                       std::vector<bool>& tokenBools)
{
   std::vector<int> intBools;
   StringUtilities::token(s, separators, intBools);

   tokenBools.clear();
   const int num = static_cast<int>(intBools.size());
   for (int i = 0; i < num; i++) {
      if (intBools[i] != 0) {
         tokenBools.push_back(1);
      }
      else {
         tokenBools.push_back(0);
      }
   }
}

/**
 * like strtok() function.
 * Split string "s" at any of the characters in "separators" and place each
 * piece in tokens.
 */
void 
StringUtilities::token(const QString& s, 
            const QString& separators,
            std::vector<QString>& tokens)
{
   tokens.clear();
   
   int start = 0;
   for (int i = 0; i < (int)s.length(); i++) {
      for (int j = 0; j < (int)separators.length(); j++) {
         if (s[i] == separators[j]) {
            QString token = s.mid(start, i - start);
            if (token.length() > 0) {
               tokens.push_back(token);
            }
            start = i + 1;
            break;
         }
      }
   }
   
   QString token = s.mid(start);
   if (token.length() > 0) {
      tokens.push_back(token);
   }
}

/**
 * Split the string "stin" with the literal string "separatorString" and
 * return in "tokens".
 */
void
StringUtilities::tokenSingleSeparator(const QString& stin,
                                      const QString& separatorString,
                                      std::vector<QString>& tokens)
{
   tokens.clear();

   QString s(stin);
   const int separatorLength = separatorString.length();
   bool done = false;
   while (done == false) {
      const int pos = s.indexOf(separatorString);
      if (pos != -1) {
         const QString t(s.mid(0, pos));
         if (t.isEmpty() == false) {
            tokens.push_back(t);
         }
         s = s.mid(pos + separatorLength); 
      }     
      else {
         done = true;
      }
   }
   if (s.isEmpty() == false) {
      tokens.push_back(s);
   }
}

/**
 * Combine all strings in "tokens" with "separator" between each pair
 */
QString 
StringUtilities::combine(const std::vector<QString>& tokens,
              const QString& separator)
{
   QString s;
   
   for (unsigned int i = 0; i < tokens.size(); i++) {
      if (i > 0) {
         s.append(separator);
      }
      s.append(tokens[i]);
   }
   return s;
}

/*
 * Combine all floats in "tokens" with "separator" between each pair
 */
QString 
StringUtilities::combine(const std::vector<float>& tokenFloats,
              const QString& separator)
{
   QString s;
   
   for (unsigned int i = 0; i < tokenFloats.size(); i++) {
      if (i > 0) {
         s.append(separator);
      }
      s.append(fromNumber(tokenFloats[i]));
   }
   return s;
}

/*
 * Combine all floats in "tokens" with "separator" between each pair
 */
QString
StringUtilities::combine(const float* tokenFloats,
                         const int numFloats,
                         const QString& separator)
{
   QString s;

   for (int i = 0; i < numFloats; i++) {
      if (i > 0) {
         s.append(separator);
      }
      s.append(fromNumber(tokenFloats[i]));
   }
   return s;
}

/**
 * Combine all doubles in "tokens" with "separator" between each pair
 */
QString 
StringUtilities::combine(const std::vector<double>& tokenDoubles,
              const QString& separator)
{
   QString s;
   
   for (unsigned int i = 0; i < tokenDoubles.size(); i++) {
      if (i > 0) {
         s.append(separator);
      }
      s.append(fromNumber(tokenDoubles[i]));
   }
   return s;
}

/**
 * Combine all ints in "tokens" with "separator" between each pair
 */
QString 
StringUtilities::combine(const std::vector<int>& tokenInts,
              const QString& separator)
{
   QString s;
   
   for (unsigned int i = 0; i < tokenInts.size(); i++) {
      if (i > 0) {
         s.append(separator);
      }
      s.append(fromNumber(tokenInts[i]));
   }
   return s;
}

/**
 * Combine all ints in "tokens" with "separator" between each pair
 */
QString
StringUtilities::combine(const int* tokenInts,
                         const int numInts,
                         const QString& separator)
{
   QString s;

   for (int i = 0; i < numInts; i++) {
      if (i > 0) {
         s.append(separator);
      }
      s.append(fromNumber(tokenInts[i]));
   }
   return s;
}

/**
 * combine "token bools" into a string delinated by "separators".
 */
QString
StringUtilities::combine(const std::vector<bool>& tokenBools,
                       const QString& separator)
{
   std::vector<int> ints;
   const int num = static_cast<int>(tokenBools.size());
   for (int i = 0; i < num; i++) {
      if (tokenBools[i]) {
         ints.push_back(1);
      }
      else {
         ints.push_back(0);
      }
   }

   return StringUtilities::combine(ints, separator);
}

/**
 * make a string lower case
 */
QString 
StringUtilities::makeLowerCase(const QString& s)
{
   return s.toLower();
/*
   QString s1(s);
   for (unsigned int i = 0; i < s1.length(); i++) {
      s1[i] = (char)tolower(s1[i]);
   }
   return s1;
*/
}

/**
 * make a string upper case
 */
QString 
StringUtilities::makeUpperCase(const QString& s)
{
   return s.toUpper();
/*
   QString s1(s);
   for (unsigned int i = 0; i < s1.length(); i++) {
      s1[i] = (char)toupper(s1[i]);
   }
   return s1;
*/
}

/**
 * Convert a string for storage on a single line in a text file.
 */
QString
StringUtilities::setupCommentForStorage(const QString& stin)
{
   // NOTE:
   //  unix ends lines with "\n"
   //  mac  ends lines with "\r"
   //  dos  ends lines with "\r\n".
   QString s = stin;
   const int num = static_cast<int>(s.length());
   for (int i = 0; i < num; i++) {
      if (s[i] == '\t') {
         s[i] = ' ';
      }
      else if (s[i] == '\r') {
         if (i < (num - 1)) {
            if (s[i+1] == '\n') {
               s[i] = ' ';  // '\n' will get handled on next iteration
            }
            else {
               s[i] = '\t';
            }
         }
         else {
            s[i] = '\t';
         }
      }
      else if (s[i] == '\n') {
         s[i] = '\t';
      }
   }
   return s;
}

/**
 * Convert a string saved for storage back to display representation.
 */
QString
StringUtilities::setupCommentForDisplay(const QString& stin)
{
   QString s(stin);
   const int num = static_cast<int>(s.length());
   for (int i = 0; i < num; i++) {
      if (s[i] == '\t') {
         s[i] = '\n';
      }
   }
   return s;
}

/**
 * Determine if the string "s1" starts with the string "s2".
 */
bool 
StringUtilities::startsWith(const QString& s1, const QString& s2)
{
   const int s1len = s1.length();
   const int s2len = s2.length();

   if (s1len < s2len) {
      return false;
   }
   
   return (s1.left(s2len) == s2);
}

/**
 * Determine if the string "s1" ends with the string "s2".
 */
bool 
StringUtilities::endsWith(const QString& s1, const QString& s2)
{
   const int s1len = s1.length();
   const int s2len = s2.length();

   bool itDoes = false;
   if (s2len <= s1len) {
      QString sub = s1.right(s2len);
      if (sub == s2) {
         itDoes = true;
      }
   }
   return itDoes;
}

/**
 * Trim the blanks and tabs from both ends of a string.
 */
QString
StringUtilities::trimWhitespace(const QString& stin)
{
   if (stin.isEmpty()) {
      return stin;
   }
   return stin.trimmed();
/*
   QString s(stin);
   const int firstNonBlank = StringUtilities::findFirstOf(s, " \t");
   if (firstNonBlank != -1) {
      s = s.mid(firstNonBlank);
   }
   else {
      s = "";
   }

   const int lastNonBlank = s.find_last_not_of(" \t");
   if (lastNonBlank != -1) {
      s.resize(lastNonBlank + 1);
   }
   
   return s;
*/
}

/**
 * right justify a string by prepending blanks to it so the
 * string contains "totalSize" characters.
 */
 QString 
 StringUtilities::rightJustify(const QString& s, 
                               const int totalSize)
{
   return s.rightJustified(totalSize);
/*
   const int len = static_cast<int>(s.length());
   
   if (len < totalSize) {
      QString str;
      str.fill(totalSize - len, ' ');
      str.append(s);
      return str;
   }
   else {
      return s;
   }
*/
}

/**
 * left justify a string by adding blanks to it so the
 * string contains "totalSize" characters.
 */
 QString 
 StringUtilities::leftJustify(const QString& s, 
                               const int totalSize)
{
   return s.leftJustified(totalSize);
/*
   const int len = static_cast<int>(s.length());
   
   if (len < totalSize) {
      QString s2;
      s2.fill(totalSize - len, ' ');
      QString str(s);
      str += s2;
      return str;
   }
   else {
      return s;
   }
*/
}

/**
 * convert http URLs to hyperlinks.
 *   "http://www.wustl.edu" becomes "<a href=http://www.wustl.edu>http://wwww.wustl.edu</a>"
 */
QString
StringUtilities::convertURLsToHyperlinks(const QString& sIn)
{
   std::vector<QString> url;
   std::vector<int> urlStart;

   if (sIn.indexOf("http://") == -1) {
      return sIn;
   }
   else {
      //
      // Create a modifiable copy
      //
      QString s(sIn);
      
      //
      // loop since there may be more than one URL
      //
      bool done = false;
      int startPos = 0;
      while(done == false) {
         //
         // Find the beginning of the URL 
         //
         const int httpStart = s.indexOf("http://", startPos); 
         
         //
         // Was the start of a URL found
         //
         if (httpStart == -1) {
            done = true;
         }
         else {
            //
            // Find the end of the URL
            //
            int httpEnd = StringUtilities::findFirstOf(s, " \t\n\r", httpStart + 1);
            
            //
            // May not find end since end of string
            //
            int httpLength;
            if (httpEnd == -1) {
               httpLength = s.length() - httpStart;
            }
            else {
               httpLength = httpEnd - httpStart;
            }
            
            //
            // Get the http URL
            //
            const QString httpString = s.mid(httpStart, httpLength);
            url.push_back(httpString);
            urlStart.push_back(httpStart);

            //
            // Prepare for next search
            //
            startPos = httpStart;
            //if (startPos > 0) {
               startPos = startPos + 1;
            //}
         }
      }

      if (url.empty() == false) {
         const int startNum = static_cast<int>(url.size()) - 1;
         for (int i = startNum; i >= 0; i--) {
            const int len = url[i].length();

            //
            // Create the trailing part of the hyperlink and insert it
            //
            QString trailingHyperLink("\">");
            trailingHyperLink.append(url[i]);
            trailingHyperLink.append("</a>");
            s.insert(urlStart[i] + len, trailingHyperLink);

            //
            // Insert the beginning of the hyperlink
            //
            s.insert(urlStart[i], " <a href=\"");
         }
      }
      return s;
   }
}

/**
 * convert a string to an unsigned byte.
 */
unsigned char
StringUtilities::toUnsignedByte(const QString& s)
{
   return static_cast<unsigned char>(s.toInt());
}

/**
 * convert a string to an unsigned byte.
 */
void 
StringUtilities::toNumber(const QString& s, unsigned char& b)
{
   b = toUnsignedByte(s);
}

/**
 * convert a unsigned byte to a string.
 */
QString 
StringUtilities::fromNumber(const unsigned char b)
{
   return QString::number(static_cast<int>(b));
}
      
/**
 * Convert an string to an integer. 
 */
int
StringUtilities::toInt(const QString& s)
{
   return s.toInt();
}

/**
 * convert a string to an int.
 */
void 
StringUtilities::toNumber(const QString& s, int& i)
{
   i = toInt(s);
}

/**
 * Convert an integer to a string.
 */
QString
StringUtilities::fromNumber(const int i)
{
   return QString::number(i);
}

/**
 * Convert an string to a float.  Handles "em" or "en" dash (various width dashes)
 */
float
StringUtilities::toFloat(const QString& sin)
{
   float f = 0.0;
   
   QString s = sin.trimmed();
   if (s.isEmpty() == false) {
      QChar c = s[0];
      //
      // Underscore
      //
      if (c == QChar('_')) {
         s[0] = '-';
      }
      //
      // Some type of unicode "en-dash", "em-dash" -- unicode characters 8208 to 8213
      //
      else if ((c.unicode() >= 0x2010) &&   // 0x2010 == 8208  variouse types of dashs
               (c.unicode() <= 0x2015)) {   // 0x2015 == 8213
         s[0] = '-';
      }
      
      //
      // Check for a failure to convert to a floating point number
      //
      bool ok = true;
      f = s.toFloat(&ok);
      if (ok == false) {
         std::cout << "WARNING: Conversion of string to floating point number failed." << std::endl;
         std::cout << "   ascii: " << s.toAscii().constData() << std::endl;
         std::cout << "   unicode (hex): ";
         for (int i = 0; i < s.length(); i++) {
            std::cout << QString::number(s[i].unicode(), 16).toAscii().constData() << " ";
         }
         std::cout << std::endl << std::endl;
      }
   }
   
   return f;
}

/**
 * Convert an string to a double.  Handles "em" or "en" dash (various width dashes)
 */
double
StringUtilities::toDouble(const QString& sin)
{
   double f = 0.0;
   
   QString s = sin.trimmed();
   if (s.isEmpty() == false) {
      QChar c = s[0];
      //
      // Underscore
      //
      if (c == QChar('_')) {
         s[0] = '-';
      }
      //
      // Some type of unicode "en-dash", "em-dash" -- unicode characters 8208 to 8213
      //
      else if ((c.unicode() >= 0x2010) &&   // 0x2010 == 8208  variouse types of dashs
               (c.unicode() <= 0x2015)) {   // 0x2015 == 8213
         s[0] = '-';
      }
      
      //
      // Check for a failure to convert to a double precision floating point number
      //
      bool ok = true;
      f = s.toDouble(&ok);
      if (ok == false) {
         std::cout << "WARNING: Conversion of string to floating point number failed." << std::endl;
         std::cout << "   ascii: " << s.toAscii().constData() << std::endl;
         std::cout << "   unicode (hex): ";
         for (int i = 0; i < s.length(); i++) {
            std::cout << QString::number(s[i].unicode(), 16).toAscii().constData() << " ";
         }
         std::cout << std::endl << std::endl;
      }
   }
   
   return f;
}

/**
 * convert a string to a float.
 */
void 
StringUtilities::toNumber(const QString& s, float& f)
{
   f = toFloat(s);
}

/**
 * Convert a float to a string.  Remove extra zeros right of decimal place.
 */
QString
StringUtilities::fromNumber(const float f)
{
   QString s = QString::number(f, 'f', StringUtilities::digitsRightOfDecimal);

   int decimal = s.indexOf(".");
   int num = s.length() - 1;

   if (decimal >= 0) {
      decimal++;
      for (int i = num; i > decimal; i--) {
         if (s[i] == '0') {
            s[i] = ' ';
         }
         else {
            break;
         }
      }
      s = s.trimmed();
   }
   return s;
}

/**
 * convert a string to a double.
 */
void 
StringUtilities::toNumber(const QString& s, double& d)
{
   d = toDouble(s);
}

/**
 * Convert a double to a string.
 */
QString
StringUtilities::fromNumber(const double f)
{
   std::ostringstream str;
   str.setf(std::ios::fixed);
   str.precision(digitsRightOfDecimal);

   str << f;
   
   return str.str().c_str();
}

/**
 * convert a string to an bool.
 */
bool 
StringUtilities::toBool(const QString& s)
{
   if (s.isEmpty() == false) {
      if ((s[0] == 'T') || (s[0] == 't')) {
         return true;
      }
   }
   
   return false;
}

/**
 * convert an bool to a string.
 */
QString 
StringUtilities::fromBool(const bool b)
{
   if (b) return "true";
   else return "false";
}

/**
 * get the white space characters.
 */
QString 
StringUtilities::getWhiteSpaceCharacters()
{
   return "\n\r\t ";
}

/**
 * set the digits right of decimal for float/string conversions.
 */
void 
StringUtilities::setFloatDigitsRightOfDecimal(const int numRightOfDecimal)
{
   digitsRightOfDecimal = numRightOfDecimal;
}      

/**
 * find the first of any of the characters in "lookForChars".
 */
int 
StringUtilities::findFirstOf(const QString& s, const QString& lookForChars,
                             const int startPos)
{
   const int len = s.length();
   const int len2 = lookForChars.length();
   for (int i = startPos; i < len; i++) {
      for (int j = 0; j < len2; j++) {
         if (s[i] == lookForChars[j]) {
            return i;
         }
      }
   }
   return -1;
}

/**
 * linewrap a string with no more than maxCharsPerLine.
 */
void 
StringUtilities::lineWrapString(const int maxCharsPerLine, QString& s)
{
   int iStart = 0;
   
   int cnt = 0;
   const int stringLength = s.length();
   int pos = iStart + cnt;
   int lastSpace = -1;
   while (pos < stringLength) {
      if (s[pos] == ' ') {
         lastSpace = pos;
      }
      if (s[pos] == '\n') {
         iStart = iStart;
      }
      if (cnt >= maxCharsPerLine) {
         if (lastSpace >= 0) {
            s[lastSpace] = '\n';
            iStart = lastSpace;
            cnt = 0;
            lastSpace = -1;
         }
      }
      cnt++;
      pos = iStart + cnt;
   }
}

/**
 * convert to a C++ standard string.
 */
std::string 
StringUtilities::toStdString(const QString& qs)
{
   std::string ss;
   const int len = qs.length();
   if (len <= 0) {
      ss = "";
   }
   else {
      ss.resize(len);
      for (int i = 0; i < len; i++) {
         ss[i] = qs[i].toAscii();
      }
   }
   
   return ss;
}

/**
 * convert from a C++ standard string.
 */
QString 
StringUtilities::fromStdString(const std::string& ss)
{
   QString qs;
   const int len = ss.length();
   if (len <= 0) {
      qs = "";
   }
   else {
      qs.resize(len);
      for (int i = 0; i < len; i++) {
         qs[i] = ss[i];
      }
   }
   
   return qs;
}      
