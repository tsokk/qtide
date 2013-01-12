#include <QStringList>
#include <QString>
#include <QDebug>

#include "font.h"

// ---------------------------------------------------------------------
Font::Font(string s)
{
  angle=0;
  QString face, sizestyle;
  int bold=0,italic=0,strikeout=0,underline=0;
  float size=1;
  QString textstring = QString::fromUtf8 (s.c_str());
  if (textstring.startsWith("\"")) {
    QStringList ss=textstring.split("\"",QString::SkipEmptyParts);
    face = ss[0];
    if (ss.length()>1) {
      sizestyle = ss[1];
      QStringList ss1=sizestyle.split(" ",QString::SkipEmptyParts);
      for (int j = 0; j < ss1.length(); j++) {
        if (ss1[j]=="bold") bold = 1;
        else if (ss1[j]=="italic") italic = 1;
        else if (ss1[j]=="underline") underline = 1;
        else if (ss1[j]=="strikeout") strikeout = 1;
        else if (ss1[j].mid(0,5)=="angle") angle = ss1[j].mid(5).toInt();
        else size = ss1[j].toFloat();
      }
    }
  } else {
    QStringList ss=textstring.split(" ",QString::SkipEmptyParts);
    face = ss[0];
    if (ss.length()>1) {
      if (ss.length()>1) {
        for (int j = 1; j < ss.length(); j++) {
          if (ss[j]=="bold") bold = 1;
          else if (ss[j]=="italic") italic = 1;
          else if (ss[j]=="underline") underline = 1;
          else if (ss[j]=="strikeout") strikeout = 1;
          else if (ss[j].mid(0,5)=="angle") angle = ss[j].mid(5).toInt();
          else size = ss[j].toFloat();
        }
      }
    }
  }

//  qDebug() << "font: " + face + ",size=" + QString::number(size) + ",bold=" + QString::number(bold) + ",italic=" + QString::number(italic) + ",strikeout=" + QString::number(strikeout) + ",underline=" + QString::number(underline) + ",angle=" + QString::number(angle) ;
  font = QFont (face);
  font.setPointSizeF(size);
  font.setBold(bold);
  font.setItalic(italic);
  font.setStrikeOut(strikeout);
  font.setUnderline(underline);
}

// ---------------------------------------------------------------------
Font::Font(string s,int size10, bool bold, bool italic, bool strikeout, bool underline, int angle10)
{
  angle=angle10;
  QString face = QString::fromUtf8 (s.c_str());
  font = QFont (face);
  font.setPointSizeF(size10/10);
  font.setBold(bold);
  font.setItalic(italic);
  font.setStrikeOut(strikeout);
  font.setUnderline(underline);
}