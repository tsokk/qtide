
#include <QCheckBox>

#include "wd.h"
#include "checkbox.h"
#include "form.h"

// ---------------------------------------------------------------------
CheckBox::CheckBox(string s, string p, Form *f) : Child(s,p,f)
{
  type="checkbox";
  QCheckBox *w=new QCheckBox;
  widget=(QWidget*) w;
  QString qs=s2q(s);
  w->setObjectName(qs);
  w->setText(qs);
  connect(w,SIGNAL(stateChanged(int)),
          this,SLOT(stateChanged()));
}

// ---------------------------------------------------------------------
void CheckBox::set(string p)
{
  QCheckBox *w=(QCheckBox*) widget;
  w->setChecked(p=="1");
}

// ---------------------------------------------------------------------
void CheckBox::setp(string p,string v)
{
  if (p=="caption")
    ((QCheckBox *)widget)->setText(s2q(v));
  else Child::setp(p,v);
}

// ---------------------------------------------------------------------
string CheckBox::state()
{
  QCheckBox *w=(QCheckBox*) widget;
  return spair(id,w->isChecked()?"1":"0");
}

// ---------------------------------------------------------------------
void CheckBox::stateChanged()
{
  event="button";
  pform->signalevent(this);
}