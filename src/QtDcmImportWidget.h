/*
    QtDcm is a C++ Qt based library for communication and conversion of Dicom images.
    Copyright (C) 2011  Alexandre Abadie <Alexandre.Abadie@univ-rennes1.fr>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef QTDCMIMPORTWIDGET_H
#define QTDCMIMPORTWIDGET_H

#include "qtdcmExports.h"
#include <QtGui>
#include "ui_qtdcmImportWidget.h"

class QTDCM_EXPORT QtDcmImportWidget : public QWidget, public Ui::QtDcmImportWidget
{
    Q_OBJECT
public:
    explicit QtDcmImportWidget ( QWidget * parent = 0 );
    virtual ~QtDcmImportWidget();
    
    void hideProgressBar();
    void showProgressBar();
    void hideProgressLabel();
    void showProgressLabel();
    void setImportButtonEnabled(bool enable);
    void setFetchMode();
    void setProgressLabelMessage(const QString &msg);
    
public slots:
  void updateProgressBar ( int i );

};

#endif // QTDCMIMPORTWIDGET_H
