#ifndef NTABS_H
#define NTABS_H

#include <QTabWidget>

class Nedit;
class Note;

class Ntabs : public QTabWidget

{
  Q_OBJECT

public:
  Ntabs(Note *);
  void projectopen(bool);
  QStringList gettablist();
  void setfont(QFont font);
  void setlinenos(bool b);
  void setlinewrap(bool b);
  void setmodified(int index,bool b);

  void tabclose(int index);
  void tabcloseall();
  void tabclosefile(QString f);

  bool tabopen(QString s,int line);
  void tabrestore(int index);
  bool tabsave(int index);
  void tabsaveall();
  void tabsaveas(int index);
  bool tabsaveOK(int index);
  void tabsetindex(int index);
  void tabsettext(QString s);

  QString Id;

private slots:
  void currentChanged(int index);

  void modificationChanged(bool b);
  void tabCloseRequested(int index);

private:
  void tabsetcolor(int index,bool ifmod);
  bool tabopen1(QString s,int line);

  Note *pnote;

};

#endif