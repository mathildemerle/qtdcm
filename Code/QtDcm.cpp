#include "QtDcm.h"
#include <QtDcmPatient.h>
#include <QtDcmStudy.h>
#include <QtDcmSerie.h>
#include <QtDcmServer.h>
#include <QtDcmImage.h>
#include <QtDcmPreferences.h>
#include <QtDcmPreferencesDialog.h>
#include <QtDcmManager.h>

class QtDcmPrivate
{

public:
    int mode;
    QList<QString> imagesList; /** Contains the images filenames of the current serie (i.e selected in the treewidget)*/
    QString currentSerieId; /** Id of the current selected serie */
    QDate beginDate, endDate; /** Begin and end for Q/R retrieve parameters */
    QMap<QString, QList<QString> > selectedSeries;
};

QtDcm::QtDcm ( QWidget *parent ) : QWidget ( parent ), d ( new QtDcmPrivate )
{
    QTextCodec::setCodecForCStrings ( QTextCodec::codecForName ( "iso" ) );
    setupUi ( this );
    d->mode = QtDcm::CD;

    //Initialize QTreeWidgetPatients
    treeWidgetPatients->setColumnWidth ( 0, 400 );
    treeWidgetPatients->setColumnWidth ( 1, 100 );
    treeWidgetPatients->setColumnWidth ( 2, 100 );
    QStringList labelsPatients;
    labelsPatients << "Patients name" << "ID" << "Birthdate" << "Sex";
    treeWidgetPatients->setHeaderLabels ( labelsPatients );
    treeWidgetPatients->setContextMenuPolicy ( Qt::CustomContextMenu );

    //Initialize QTreeWidgetSeries
    treeWidgetStudies->setColumnWidth ( 0, 200 );
    treeWidgetStudies->setColumnWidth ( 1, 100 );
    QStringList labelsStudies;
    labelsStudies << "Studies description" << "Date" << "ID";
    treeWidgetStudies->setHeaderLabels ( labelsStudies );
    treeWidgetStudies->setContextMenuPolicy ( Qt::CustomContextMenu );

    //Initialize QTreeWidgetSeries
    treeWidgetSeries->setColumnWidth ( 0, 230 );
    treeWidgetSeries->setColumnWidth ( 1, 100 );
    treeWidgetSeries->setColumnWidth ( 2, 100 );
    QStringList labelsSeries;
    labelsSeries << "Series description" << "Modality" << "Date" << "ID";
    treeWidgetSeries->setHeaderLabels ( labelsSeries );
    treeWidgetSeries->setContextMenuPolicy ( Qt::CustomContextMenu );

    //Initialize widgets
    startDateEdit->setDate ( QDate ( 1900, 01, 01 ) );
    endDateEdit->setDate ( QDate::currentDate() );

    QtDcmManager::instance()->setPatientsTreeWidget ( treeWidgetPatients );
    QtDcmManager::instance()->setStudiesTreeWidget ( treeWidgetStudies );
    QtDcmManager::instance()->setSeriesTreeWidget ( treeWidgetSeries );

    QtDcmManager::instance()->setDate1 ( startDateEdit->date().toString ( "yyyyMMdd" ) );
    QtDcmManager::instance()->setDate2 ( endDateEdit->date().toString ( "yyyyMMdd" ) );

    this->updatePacsComboBox();
    initConnections();
}

QtDcm::~QtDcm()
{
//     delete QtDcmManager::instance();
}

// QtDcmManager * QtDcm::getManager()
// {
//     return QtDcmManager::instance();
// }

void QtDcm::initConnections()
{
    // Initialize connections
    QObject::connect ( treeWidgetPatients, SIGNAL ( currentItemChanged ( QTreeWidgetItem*, QTreeWidgetItem* ) ), this, SLOT ( patientItemSelected ( QTreeWidgetItem*, QTreeWidgetItem* ) ) );
    QObject::connect ( treeWidgetStudies, SIGNAL ( currentItemChanged ( QTreeWidgetItem*, QTreeWidgetItem* ) ), this, SLOT ( studyItemSelected ( QTreeWidgetItem*, QTreeWidgetItem* ) ) );
    QObject::connect ( treeWidgetSeries, SIGNAL ( currentItemChanged ( QTreeWidgetItem*, QTreeWidgetItem* ) ), this, SLOT ( serieItemSelected ( QTreeWidgetItem*, QTreeWidgetItem* ) ) );
    QObject::connect ( treeWidgetSeries, SIGNAL ( itemClicked ( QTreeWidgetItem*, int ) ), this, SLOT ( serieItemClicked ( QTreeWidgetItem*, int ) ) );
    QObject::connect ( nameEdit, SIGNAL ( textChanged ( QString ) ), this, SLOT ( patientNameTextChanged ( QString ) ) );
    QObject::connect ( serieDescriptionEdit, SIGNAL ( textChanged ( QString ) ), this, SLOT ( serieDescriptionTextChanged ( QString ) ) );
    QObject::connect ( studyDescriptionEdit, SIGNAL ( textChanged ( QString ) ), this, SLOT ( studyDescriptionTextChanged ( QString ) ) );
    QObject::connect ( searchButton, SIGNAL ( clicked() ), this, SLOT ( findSCU() ) );
    QObject::connect ( cdromButton, SIGNAL ( clicked() ), this, SLOT ( openDicomdir() ));
    QObject::connect ( patientSexComboBox, SIGNAL ( currentIndexChanged ( int ) ), this, SLOT ( updateSex ( int ) ) );
    QObject::connect ( serieModalityComboBox, SIGNAL ( currentIndexChanged ( int ) ), this, SLOT ( updateModality ( int ) ) );
    QObject::connect ( pacsComboBox, SIGNAL ( currentIndexChanged ( int ) ), this, SLOT ( updatePACS ( int ) ) );
    QObject::connect ( startDateEdit, SIGNAL ( dateChanged ( QDate ) ), this, SLOT ( startDateChanged ( QDate ) ) );
    QObject::connect ( endDateEdit, SIGNAL ( dateChanged ( QDate ) ), this, SLOT ( endDateChanged ( QDate ) ) );
}

void QtDcm::updatePacsComboBox()
{
    pacsComboBox->blockSignals ( true );
    pacsComboBox->clear();

    for ( int i = 0; i < QtDcmManager::instance()->getPreferences()->getServers().size(); i++ )
    {
        pacsComboBox->addItem ( QtDcmManager::instance()->getPreferences()->getServers().at ( i )->getName() );
    }
    pacsComboBox->blockSignals ( false );
}

void QtDcm::findSCU()
{
    d->mode = QtDcm::PACS;
    treeWidgetPatients->clear();
    treeWidgetStudies->clear();
    treeWidgetSeries->clear();
    QtDcmManager::instance()->findPatientsScu();
}

void QtDcm::clearDisplay()
{
    treeWidgetPatients->clear();
    treeWidgetStudies->clear();
    treeWidgetSeries->clear();
}

void QtDcm::patientItemSelected ( QTreeWidgetItem* current, QTreeWidgetItem* previous )
{
    QtDcmManager::instance()->clearSerieInfo();
    QtDcmManager::instance()->clearPreview();

    treeWidgetStudies->clear();

    if ( current != 0 )   // Avoid crash when clearDisplay is called
    {
        if ( d->mode == QtDcm::PACS )
            QtDcmManager::instance()->findStudiesScu ( current->text ( 0 ) );
        else
            QtDcmManager::instance()->findStudiesDicomdir ( current->text ( 0 ) );
    }
}

void QtDcm::studyItemSelected ( QTreeWidgetItem* current, QTreeWidgetItem* previous )
{
    treeWidgetSeries->clear();

    QtDcmManager::instance()->clearSerieInfo();
    QtDcmManager::instance()->clearPreview();

    if ( current != 0 )   // Avoid crash when clearDisplay is called
    {
        if ( d->mode == QtDcm::PACS )
            QtDcmManager::instance()->findSeriesScu ( treeWidgetPatients->currentItem()->text ( 0 ), current->text ( 0 ) );
        else
            QtDcmManager::instance()->findSeriesDicomdir ( treeWidgetPatients->currentItem()->text ( 0 ), current->text ( 0 ) );
    }
}

void QtDcm::serieItemSelected ( QTreeWidgetItem* current, QTreeWidgetItem* previous )
{
    if ( current != 0 )   // Avoid crash when clearDisplay is called
    {
        if ( d->mode == QtDcm::CD )
            QtDcmManager::instance()->findImagesDicomdir ( current->text ( 3 ) );

        QtDcmManager::instance()->updateSerieInfo(current->data ( 4, 0 ).toString(), current->data ( 5, 0 ).toString(), current->data ( 6, 0 ).toString());

        int elementCount = current->data ( 4, 0 ).toInt();
        QtDcmManager::instance()->clearPreview();
        QtDcmManager::instance()->getPreviewFromSelectedSerie ( current->text ( 3 ), elementCount / 2 );
    }

//     detailsFrame->show();
}

void QtDcm::serieItemClicked ( QTreeWidgetItem * item, int column )
{
    if ( item->checkState ( 0 ) == Qt::Checked )
        QtDcmManager::instance()->addSerieToImport ( item->text ( 3 ) );
    else
        QtDcmManager::instance()->removeSerieToImport ( item->text ( 3 ) );
}

void QtDcm::openDicomdir()
{
    this->clearDisplay();
    d->mode = QtDcm::CD;
    // Open a QFileDialog for choosing a Dicomdir
    QFileDialog dialog ( this );
    dialog.setFileMode ( QFileDialog::ExistingFile );
    dialog.setDirectory ( QDir::home().dirName() );
    dialog.setWindowTitle ( tr ( "Open dicomdir" ) );
    dialog.setNameFilter ( tr ( "Dicomdir (dicomdir DICOMDIR)" ) );
    QString fileName;

    if ( dialog.exec() )
    {
        fileName = dialog.selectedFiles() [0];
    }

    dialog.close();

    if ( !fileName.isEmpty() )   // A file has been chosen
    {
        QtDcmManager::instance()->setDicomdir ( fileName );
        this->loadPatientsFromDicomdir();
    }
}

void QtDcm::loadPatientsFromDicomdir()
{
    this->clearDisplay();
    QtDcmManager::instance()->loadDicomdir();
}

void QtDcm::queryPACS()
{
    this->findSCU();
}

void QtDcm::updateModality ( int index )
{
    switch ( index )
    {

    case 0://MR
        QtDcmManager::instance()->setModality ( "*" );
        break;

    case 1://CT
        QtDcmManager::instance()->setModality ( "MR" );
        break;

    case 2://US
        QtDcmManager::instance()->setModality ( "CT" );
        break;

    case 3://PET
        QtDcmManager::instance()->setModality ( "PET" );
        break;
    }

    treeWidgetSeries->clear();

    if ( treeWidgetPatients->currentItem() && treeWidgetStudies->currentItem() )
    {
        if ( d->mode == QtDcm::PACS )
            QtDcmManager::instance()->findSeriesScu ( treeWidgetPatients->currentItem()->text ( 0 ), treeWidgetStudies->currentItem()->text ( 0 ) );
        else
            qDebug() << "recherche sur le cd";
    }
}

void QtDcm::updateSex ( int index )
{
    switch ( index )
    {

    case 0://all
        QtDcmManager::instance()->setPatientSex ( "*" );
        this->queryPACS();
        break;

    case 1://M
        QtDcmManager::instance()->setPatientSex ( "M" );
        this->queryPACS();
        break;

    case 2://F
        QtDcmManager::instance()->setPatientSex ( "F" );
        this->queryPACS();
    }
}

void QtDcm::updatePACS ( int index )
{
    QtDcmManager::instance()->setCurrentPacs ( index );
    this->findSCU();
}

void QtDcm::startDateChanged ( QDate date )
{
    if ( date > endDateEdit->date() )
    {
        date = endDateEdit->date();
        startDateEdit->setDate ( date );
        return;
    }

    QtDcmManager::instance()->setDate1 ( date.toString ( "yyyyMMdd" ) );

    treeWidgetStudies->clear();
    treeWidgetSeries->clear();

    if ( treeWidgetPatients->currentItem() )
    {
        if ( d->mode == QtDcm::PACS )
            QtDcmManager::instance()->findStudiesScu ( treeWidgetPatients->currentItem()->text ( 0 ) );
        else
            qDebug() << "recherche sur le cd";
    }
}

void QtDcm::endDateChanged ( QDate date )
{
    if ( date < startDateEdit->date() )
    {
        date = startDateEdit->date();
        endDateEdit->setDate ( date );
        return;
    }

    QtDcmManager::instance()->setDate2 ( date.toString ( "yyyyMMdd" ) );

    treeWidgetStudies->clear();
    treeWidgetSeries->clear();

    if ( treeWidgetPatients->currentItem() )
    {
        if ( d->mode == QtDcm::PACS )
            QtDcmManager::instance()->findStudiesScu ( treeWidgetPatients->currentItem()->text ( 0 ) );
        else
            qDebug() << "recherche sur le cd";
    }
}

void QtDcm::editPreferences()
{
    //Launch a dialog window for editing PACS settings
    QtDcmPreferencesDialog * dialog = new QtDcmPreferencesDialog ( this );
    dialog->getWidget()->setPreferences ( QtDcmManager::instance()->getPreferences() );
    dialog->setPreferences(QtDcmManager::instance()->getPreferences());

    if ( dialog->exec() )
    {
        dialog->getWidget()->updatePreferences();
        dialog->updatePreferences();
//         this->updatePacsComboBox();
    }
    dialog->close();
    delete dialog;
}

void QtDcm::patientNameTextChanged ( QString name )
{
    if ( name.isEmpty() )
        QtDcmManager::instance()->setPatientName ( "*" );
    else
        QtDcmManager::instance()->setPatientName ( name + "*" );

    if ( d->mode == QtDcm::PACS )
        this->findSCU();
}

void QtDcm::studyDescriptionTextChanged ( QString desc )
{
    if ( desc.isEmpty() )
        QtDcmManager::instance()->setStudyDescription ( "*" );
    else
        QtDcmManager::instance()->setStudyDescription ( "*" + desc + "*" );

    if ( d->mode == QtDcm::PACS )
    {
        treeWidgetStudies->clear();
        treeWidgetSeries->clear();
    }
    if ( treeWidgetPatients->currentItem() )
    {
        if ( d->mode == QtDcm::PACS )
            QtDcmManager::instance()->findStudiesScu ( treeWidgetPatients->currentItem()->text ( 0 ) );
    }
}

void QtDcm::serieDescriptionTextChanged ( QString desc )
{
    if ( desc.isEmpty() )
    {
        QtDcmManager::instance()->setSerieDescription ( "*" );
    }
    else
        QtDcmManager::instance()->setSerieDescription ( "*" + desc + "*" );

    if ( d->mode == QtDcm::PACS )
        treeWidgetSeries->clear();

    if ( treeWidgetPatients->currentItem() && treeWidgetStudies->currentItem() )
    {
        if ( d->mode == QtDcm::PACS )
            QtDcmManager::instance()->findSeriesScu ( treeWidgetPatients->currentItem()->text ( 0 ), treeWidgetStudies->currentItem()->text ( 0 ) );
    }
}
