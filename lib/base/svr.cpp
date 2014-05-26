/* J svr */

#include <QApplication>
#include <QDataStream>
#include <QEventLoop>
#include <QString>
#include <QTimer>

#include <csignal>
#ifdef _WIN32
#else
#include <unistd.h>
#endif

#include "base.h"
#include "base.h"
#include "jsvr.h"
#include "svr.h"
#include "tedit.h"
#include "term.h"

// output type
#define MTYOFM		1	/* formatted result array output */
#define MTYOER		2	/* error output */
#define MTYOLOG		3	/* output log */
#define MTYOSYS		4	/* system assertion failure */
#define MTYOEXIT	5	/* exit */
#define MTYOFILE	6	/* output 1!:2[2 */

using namespace std;

C* _stdcall Jinput(J jt, C*);
void _stdcall Joutput(J jt, int type, C* s);

static bool ifcmddo=false;
static bool inputready=false;
static QString inputx;
static bool logged=false;
static bool quitx=false;

void logbin(const char*s,int n);
void logcs(char *msg);
Jcon *jcon=0;
QEventLoop *evloop;
int cnt=0;

// ---------------------------------------------------------------------
void Jcon::cmd(QString s)
{
  jedo((char *)q2s(s).c_str());
}

// ---------------------------------------------------------------------
QString Jcon::cmdr(QString s)
{
  return s2q(dors(q2s(s)));
}

// ---------------------------------------------------------------------
void Jcon::cmddo(QString s)
{
  ifcmddo=true;
  jedo((char *)q2s(s).c_str());
}

// ---------------------------------------------------------------------
int Jcon::exec()
{
  QString s;

  while(1) {
    cnt++;
    jinput((char *)"   ");
    if (quitx) break;
    while(!Sentence.isEmpty()) {
      s=Sentence.at(0);
      Sentence.removeFirst();
      if ((int)sizeof(inputline)<s.size()) exit(100);
      strcpy(inputline,q2s(s).c_str());
      jedo(inputline);
    }
  }
  jefree();
  return 0;
}

// ---------------------------------------------------------------------
int Jcon::init(int argc, char* argv[])
{

  void* callbacks[] = {(void*)Joutput,0,(void*)Jinput,0,(void*)SMCON};
  int type;

  evloop=new QEventLoop();

  jepath(argv[0]);     // get path to JFE folder
  jt=jeload(callbacks);
  if(!jt) {
    char m[1000];
    jefail(m), fputs(m,stdout);
    exit(1);
  }

  adadbreak=(char**)jt; // first address in jt is address of breakdata
  signal(SIGINT,sigint);

#if defined(QT_OS_ANDROID)
  Q_UNUSED(argc);
  Q_UNUSED(type);
  *inputline=0;
  jefirst(0,(char *)",<'jqt'");
#else
  if(argc==2&&!strcmp(argv[1],"-jprofile"))
    type=3;
  else if(argc>2&&!strcmp(argv[1],"-jprofile"))
    type=1;
  else
    type=0;
  addargv(argc,argv,inputline+strlen(inputline));
  jefirst(type,inputline);
#endif

  return 0;
}

// ---------------------------------------------------------------------
// run command
void Jcon::immex(QString s)
{
  Sentence.append(s);
  QTimer *timer = new QTimer(this);
  timer->setSingleShot(true);
  connect(timer, SIGNAL(timeout()), jcon, SLOT(input()));
  timer->start();
}

// ---------------------------------------------------------------------
void Jcon::input()
{
  ifcmddo=false;
  evloop->exit();
}

// ---------------------------------------------------------------------
void Jcon::quit()
{
  quitx=true;
  input();
}

// ---------------------------------------------------------------------
void Jcon::set(QString s, QString t)
{
  sets(s,q2s(t));
}

// ---------------------------------------------------------------------
//J calls for input (debug suspension and 1!:1[1) and we call for input
char* Jcon::jinput(char* p)
{
  Q_ASSERT(tedit);
  tedit->prompt=c2q(p);
  tedit->setprompt();
  inputready=false;
  logged=true;
  evloop->exec(QEventLoop::AllEvents|QEventLoop::WaitForMoreEvents);
  return (char*) 0;
}

// ---------------------------------------------------------------------
//J calls for input (debug suspension and 1!:1[1) and we call for input
char* _stdcall Jinput(J jt, char* p)
{
  Q_UNUSED(jt);

  Q_ASSERT(tedit);
  tedit->prompt=c2q(p);
  tedit->setprompt();
  inputready=false;
  logged=true;
  evloop->exec(QEventLoop::AllEvents|QEventLoop::WaitForMoreEvents);
  QString s=jcon->Sentence.at(0);
  jcon->Sentence.removeFirst();
  if ((int)sizeof(inputline)<s.size()) exit(100);
  strcpy(inputline,q2s(s).c_str());
  return inputline;
}

// ---------------------------------------------------------------------
// J calls for output
// logged isn't used

void _stdcall Joutput(J jt,int type, char* s)
{
  Q_UNUSED(jt);

  if(MTYOEXIT==type) {
    exit((int)(intptr_t)s);
  }

  Q_ASSERT(tedit);
  int n=(int)strlen(s);
  if (n==0) return;
  if (s[n-1]=='\n') s[n-1]='\0';
  QString t=QString::fromUtf8(s);

  if (MTYOFILE==type && ifcmddo)
    tedit->append_smoutput(t);
  else if (MTYOLOG!=type)
    tedit->append(t);
  else {
    if (logged) {
      tedit->append(t);
    } else {
      logged=true;
      tedit->append("");
    }
  }
}

// ---------------------------------------------------------------------
bool svr_init(int argc, char* argv[])
{
  jcon=new Jcon();
  int r=jcon->init(argc,argv);
  if (r)
    info("Server","svr_init result: " + QString::number(r));
  return r==0;
}
