
#include <QApplication>
#include <QBoxLayout>
#include <QMenuBar>
#include <QSignalMapper>
#include <QTimer>

#include <sstream>  //include this to use string streams

#include "wd.h"
#include "font.h"
#include "cmd.h"
#include "form.h"
#include "pane.h"
#include "tabs.h"
#include "child.h"
#include "isigraph2.h"
#include "menus.h"
#ifndef QT_NO_OPENGL
#include "opengl2.h"
#endif
#include "../base/state.h"
#include "../base/term.h"

extern bool standAlone;

// ---------------------------------------------------------------------
Form::Form(string s, string p, string loc, QWidget *parent) : QWidget (parent)
{
  Q_UNUSED(parent);
  id=s;
  child=0;
  evtchild=0;
  locale=loc;
  menubar=0;
  tab=0;
  closed=false;
  shown=false;
  setAttribute(Qt::WA_DeleteOnClose);
  QStringList m=s2q(p).split(' ',QString::SkipEmptyParts);
  escclose=m.contains("escclose");
  closeok=m.contains("closeok");
  setpn(s);

  Qt::WindowFlags flags=0;
  if (m.contains("dialog")) flags=Qt::Dialog|Qt::WindowTitleHint|Qt::WindowStaysOnTopHint|Qt::CustomizeWindowHint;
  if (m.contains("popup")) flags=Qt::Popup;
  if (m.contains("minbutton")) flags|=Qt::WindowMinimizeButtonHint;
  if (m.contains("maxbutton")) flags|=Qt::WindowMaximizeButtonHint;
  if (m.contains("closebutton")) flags|=Qt::WindowCloseButtonHint;
  if (m.contains("ptop")) flags=Qt::WindowStaysOnTopHint;
  if (m.contains("owner")) {
    flags|=Qt::Window;
    setWindowModality(Qt::WindowModal);
  }
  setWindowFlags(flags);

  layout=new QVBoxLayout(this);
  setpadding(0,0,0,0);
  layout->setSpacing(0);
  if (m.contains("nosize")) layout->setSizeConstraint( QLayout::SetFixedSize );
  addpane(0);
  signalMapper=new QSignalMapper(this);
  connect(signalMapper,SIGNAL(mapped(QWidget *)),
          this,SLOT(buttonClicked(QWidget *)));
  timer=new QTimer;
  connect(timer, SIGNAL(timeout()),this,SLOT(systimer()));
}

// ---------------------------------------------------------------------
Form::~Form()
{
  Child *n;
  for (int i=0; i<children.size(); i++)
    if ("isigraph0"==(n=children.at(i))->type) {
      if ((Isigraph2*)n->widget) {
        if (((Isigraph2*)n->widget)->painter) {
          ((Isigraph2*)n->widget)->painter->end();
          delete ((Isigraph2*)n->widget)->painter;
          ((Isigraph2*)n->widget)->painter=0;
        }
      }
#ifndef QT_NO_OPENGL
    } else if ("opengl"==(n=children.at(i))->type) {
      if ((Opengl2*)n->widget) {
        if (((Opengl2*)n->widget)->painter) {
          ((Opengl2*)n->widget)->painter->end();
          delete ((Opengl2*)n->widget)->painter;
          ((Opengl2*)n->widget)->painter=0;
        }
      }
#endif
    }
  if (this==form) form = 0;
  if (this==evtform) evtform = 0;
  if (timer) delete timer;
  Forms.removeOne(this);
  if (Forms.isEmpty()) form=0;
#ifdef QT_OS_ANDROID
  if (!Forms.isEmpty()) {
    form=Forms.last();
    wdactivateform();
  }
#endif
  if (Forms.isEmpty() && (!ShowIde)) {
#ifdef QT_OS_ANDROID
    state_quit();
    QApplication::quit();
#else
    if (standAlone) {
      state_quit();
      QApplication::quit();
    } else {
      showide(true);
      if (ShowIde) {
        term->activateWindow();
        term->raise();
        term->repaint();
      }
    }
#endif
  }
}

// ---------------------------------------------------------------------
void Form::addchild(Child *c)
{
  child=c;
  children.append(c);
}

// ---------------------------------------------------------------------
void Form::addmenu()
{
  menubar= new Menus("menu","",this,0);
  addchild((Child *) menubar);
  layout->insertWidget(0,child->widget);
}

// ---------------------------------------------------------------------
Pane *Form::addpane(int n)
{
  pane=new Pane(n,this);
  panes.append(pane);
  return pane;
}

// ---------------------------------------------------------------------
void Form::buttonClicked(QWidget *w)
{
  Child *child=(Child *) w;
  child->event="button";
  signalevent(child);
}

// ---------------------------------------------------------------------
void Form::closeEvent(QCloseEvent *e)
{
  if (closeok || closed) {
    e->accept();
    return;
  }
  e->ignore();
  event="close";
  fakeid="";
  form=this;
  signalevent(0);
  if (closed) {
    e->accept();
  } else e->ignore();
}

// ---------------------------------------------------------------------
// close if not the main pane
void Form::closepane()
{
  if (panes.size()<=1) return;
  panes.removeLast();
  pane=panes.last();
}

// ---------------------------------------------------------------------
string Form::getsysmodifiers()
{
  Qt::KeyboardModifiers mod = QApplication::keyboardModifiers();
  return i2s((mod.testFlag(Qt::ShiftModifier) ? 1 : 0) +
             (mod.testFlag(Qt::ControlModifier)? 2 : 0));
}

// ---------------------------------------------------------------------
string Form::hschild()
{
  return p2s((void *)child);
}

// ---------------------------------------------------------------------
string Form::hsform()
{
  return p2s((void *)this);
}

// ---------------------------------------------------------------------
string Form::qform()
{
  return i2s(pos().x())+" "+i2s(pos().y())+" "+i2s(size().width())+" "+i2s(size().height());
}

// ---------------------------------------------------------------------
Child *Form::id2child(string n)
{
  for (int i=0; i<children.size(); i++)
    if ("menu"!=children.at(i)->type && n==children.at(i)->id)
      return children.at(i);
  return (Child *) 0;
}

// ---------------------------------------------------------------------
bool Form::ischild(Child *n)
{
  return children.contains(n);
}


// ---------------------------------------------------------------------
void Form::keyPressEvent(QKeyEvent *e)
{
  int k=e->key();
#ifdef QT_OS_ANDROID
  if (k==Qt::Key_Back) {
    QWidget::keyPressEvent(e);
    return;
  }
#endif
  if (k==Qt::Key_Escape) {
    e->ignore();
    if (closed) return;
    if (escclose) {
      if (closeok) {
        closed=true;
        close();
      } else  {
        event="close";
        fakeid="";
        form=this;
        signalevent(0);
      }
    } else {
      event="cancel";
      fakeid="";
      form=this;
      signalevent(0);
    }
  } else if (k>=Qt::Key_F1 && k<=Qt::Key_F35) {
    event="fkey";
    form=this;
    signalevent(0,e);
  } else if (k>=Qt::Key_A && k<=Qt::Key_Z && (e->modifiers() & Qt::ControlModifier)) {
    event="fkey";
    form=this;
    signalevent(0,e);
  } else
    QWidget::keyPressEvent(e);
}

// ---------------------------------------------------------------------
void Form::keyReleaseEvent(QKeyEvent *e)
{
#ifdef QT_OS_ANDROID
  if (e->key()==Qt::Key_Back) {
    if (closed) return;
    if (closeok) {
      closed=true;
      close();
    } else {
      event="close";
      fakeid="";
      form=this;
      signalevent(0);
    }
  } else QWidget::keyReleaseEvent(e);
#else
  QWidget::keyReleaseEvent(e);
#endif
}

// ---------------------------------------------------------------------
Child *Form::setmenuid(string id)
{
  if (menubar && menubar->items.contains(s2q(id)))
    return (Child *) menubar;
  return (Child *) 0;
}

// ---------------------------------------------------------------------
void Form::setpadding(int l,int t,int r,int b)
{
  layout->setContentsMargins(l,t,r,b);
}

// ---------------------------------------------------------------------
void Form::setpn(string p)
{
  setWindowTitle(s2q(p));
}

// ---------------------------------------------------------------------
void Form::setpicon(string p)
{
  QString s=s2q(p);
  setWindowIcon(QIcon(s));
}

// ---------------------------------------------------------------------
void Form::settimer(string p)
{
  int n=c_strtoi(p);
  if (n)
    timer->start(n);
  else
    timer->stop();
}

// ---------------------------------------------------------------------
void Form::showit()
{
#ifdef QT_OS_ANDROID
// showide(false);
  if (Forms.size()>1)
    (Forms.at(Forms.size()-2))->setVisible(false);
#endif
  for (int i=tabs.size()-1; i>=0; i--)
    tabs.last()->tabend();
  for (int i=panes.size()-1; i>=0; i--)
    panes.last()->fini();
  layout->addWidget(pane);
  setLayout(layout);
  show();

  shown=true;
}

// ---------------------------------------------------------------------
void Form::signalevent(Child *c, QKeyEvent *e)
{
  if (NoEvents || closed) return;
  string loc = locale;
  evtform=this;
  if (c) {
    evtchild=c;
    c->setform();
    sysmodifiers=c->sysmodifiers;
    if (sysmodifiers.empty())
      sysmodifiers=getsysmodifiers();
    sysdata=c->sysdata;
    loc = (""!=c->locale)?c->locale:locale;
  } else {
    evtchild=0;
    if (event=="fkey") {
      int k=e->key();
      if (k>=Qt::Key_A && k<=Qt::Key_Z && (e->modifiers() & Qt::ControlModifier)) {
        fakeid=(char)e->key()+32;  // lower case
        fakeid=fakeid + "ctrl" + string( (e->modifiers() & Qt::ShiftModifier) ? "shift" : "" );
      } else if (k>=Qt::Key_F1 && k<=Qt::Key_F35) {
        ostringstream ostr;
        ostr << e->key()+1-Qt::Key_F1;
        fakeid="f"+ ostr.str() + string((e->modifiers() & Qt::ControlModifier) ? "ctrl" : "") + string((e->modifiers() & Qt::ShiftModifier) ? "shift" : "");
      }
    }
  }
  var_cmddo("(i.0 0)\"_ wdhandler_" + s2q(loc) + "_$0");
}

// ---------------------------------------------------------------------
string Form::state(int evt)
{
  string c,c1,e,r,s,ec;

  if (evt) {
    if (evtchild) {
      c=evtchild->eid;
      e=evtchild->event;
      ec=evtchild->locale;
    } else {
      c=fakeid;
      e=event;
    }
    c1=(c.empty()) ? string("") : (c+"_") ;
    r+=spair("syshandler",id+"_handler");
    r+=spair("sysevent",id+"_"+c1+e);
    r+=spair("sysdefault",id+"_default");
    r+=spair("sysparent",id);
    r+=spair("syschild",c);
    r+=spair("systype",e);
    r+=spair("syslocalec",ec);
  }

  // need only syslocale (not syslocalep, syslocalec)?...  in isigraph
  r+=spair("syslocalep",locale);
  r+=spair("syshwndp",hsform());
  r+=spair("syshwndc",hschild());
  r+=spair("syslastfocus",(string)"");
  r+=spair("sysmodifiers",sysmodifiers);
  r+=spair("sysdata",sysdata);

  for (int i=0; i<children.size(); i++)
    s+=children.at(i)->state();

  return r+s;
}

// ---------------------------------------------------------------------
// for debugging
void Form::status(string s)
{
  qDebug() << "form status: " << s2q(s);
  qDebug() << "current pane, panes: " << pane << panes;
  qDebug() << "current tab, tabs: " << tab << tabs;
}

// ---------------------------------------------------------------------
void Form::systimer()
{
  event="timer";
  fakeid="";
  signalevent(0);
}
