#include <cmath>
#include "BarbSubtabs.h"
#include "vapor/BarbParams.h"

#define X 0
#define Y 1
#define Z 2

#define COUNT_MIN 1
#define COUNT_MAX 50

#define LENGTH_MIN .01
#define LENGTH_MAX 4
#define THICKNESS_MIN .01
#define THICKNESS_MAX 4

BarbVariablesSubtab::BarbVariablesSubtab(QWidget* parent) {
	setupUi(this);

	VariablesWidget::DisplayFlags displayFlags;
	displayFlags  = (VariablesWidget::DisplayFlags)
	    (VariablesWidget::VECTOR | VariablesWidget::HGT |
	    VariablesWidget::COLOR);

	VariablesWidget::DimFlags dimFlags;
	dimFlags = (VariablesWidget::DimFlags)
	    (VariablesWidget::TWOD | VariablesWidget::THREED);

	_variablesWidget->Reinit(displayFlags, dimFlags);
}

void BarbVariablesSubtab::Update(
	VAPoR::DataMgr *dataMgr,
	VAPoR::ParamsMgr *paramsMgr,
	VAPoR::RenderParams *rParams
) {
	_variablesWidget->Update(dataMgr, paramsMgr, rParams);
}

void BarbVariablesSubtab::Initialize(VAPoR::BarbParams* bParams,
								VAPoR::DataMgr* dataMgr) {
}

BarbGeometrySubtab::BarbGeometrySubtab(QWidget* parent) {
	setupUi(this);
	_geometryWidget->Reinit(
		GeometryWidget::MINMAX,
		GeometryWidget::VECTOR);
}

void BarbGeometrySubtab::Update(
	VAPoR::ParamsMgr *paramsMgr,
	VAPoR::DataMgr *dataMgr,
	VAPoR::RenderParams *rParams
) {
	_bParams = (VAPoR::BarbParams*)rParams;
	_geometryWidget->Update(paramsMgr, dataMgr, rParams);
	_copyRegionWidget->Update(paramsMgr, rParams);
	_transformTable->Update(rParams->GetTransform());
}

BarbAppearanceSubtab::BarbAppearanceSubtab(QWidget* parent) {
	setupUi(this);
	_TFWidget->Reinit((TFWidget::Flags)
		(TFWidget::COLORVAR | TFWidget::CONSTANT));

	_xDimCombo = new Combo(xDimEdit, xDimSlider, true);
	_yDimCombo = new Combo(yDimEdit, yDimSlider, true);
	_lengthCombo = new Combo(lengthScaleEdit, lengthScaleSlider, false);
	_thicknessCombo = new Combo(thicknessEdit, thicknessSlider, false);

	_zDimSelector->SetLabel( QString::fromAscii("Z Dimension") );
	_zDimSelector->SetIntType( true );
	_zDimSelector->SetExtents(COUNT_MIN, COUNT_MAX);

	connect(_xDimCombo, SIGNAL(valueChanged(int)), this,
		SLOT(xDimChanged(int)));
	connect(_yDimCombo, SIGNAL(valueChanged(int)), this,
		SLOT(yDimChanged(int)));
	connect(_zDimSelector, SIGNAL(valueChanged(int)), this,
		SLOT(zDimChanged(int)));
	connect(_lengthCombo, SIGNAL(valueChanged(double)), this,
		SLOT(lengthChanged(double)));
	connect(_thicknessCombo, SIGNAL(valueChanged(double)), this,
		SLOT(thicknessChanged(double)));
	connect(_recalcScalesButton, SIGNAL(pressed()), this,
		SLOT(recalculateScales()));
}

void BarbAppearanceSubtab::_hideZDimWidgets() {
	_zDimSelector->hide();
	tab->adjustSize();
	BarbLayoutTab->adjustSize();
	adjustSize();
}

void BarbAppearanceSubtab::_showZDimWidgets() {
	_zDimSelector->show();
	tab->adjustSize();
	BarbLayoutTab->adjustSize();
	adjustSize();
}

bool BarbAppearanceSubtab::_isVariable2D() const {
	VAPoR::Grid* grid;
	int ts, level, lod;
	std::vector<string> varNames = _bParams->GetFieldVariableNames();

	for (int i=0; i<3; i++) {
		string varName = varNames[i];
		if (varName=="") {
			continue;
		}

		ts    = _bParams->GetCurrentTimestep();
		level = _bParams->GetRefinementLevel();
		lod   = _bParams->GetCompressionLevel();
		grid  = _dataMgr->GetVariable(ts, varName, level, lod);

		int dimSize = grid->GetDimensions().size();
		if (dimSize == 2)
			return true;		
	}

	return false;
}

void BarbAppearanceSubtab::Update(VAPoR::DataMgr* dataMgr,
							VAPoR::ParamsMgr* paramsMgr,
							VAPoR::RenderParams* bParams) {
	_dataMgr = dataMgr;
	_bParams = (VAPoR::BarbParams*)bParams;
	_paramsMgr = paramsMgr;
	_TFWidget->Update(dataMgr, paramsMgr, bParams);
	_ColorbarWidget->Update(dataMgr, paramsMgr, bParams);

	vector<long> grid = _bParams->GetGrid();
	_xDimCombo->Update(COUNT_MIN, COUNT_MAX, grid[X]);
	_yDimCombo->Update(COUNT_MIN, COUNT_MAX, grid[Y]);

	_zDimSelector->SetValue(grid[Z]);

	vector<double> minExt, maxExt;
	double length = _bParams->GetLengthScale();
	_lengthCombo->Update(LENGTH_MIN, LENGTH_MAX, length);

	double thickness = _bParams->GetLineThickness();
	_thicknessCombo->Update(THICKNESS_MIN, THICKNESS_MAX, thickness);
	
	if (_isVariable2D())
		_hideZDimWidgets();
	else
		_showZDimWidgets();
}

void BarbAppearanceSubtab::xDimChanged(int i) {
	vector<long> longDims = _bParams->GetGrid();
	int dims[3];
	
	dims[X] = i;  
	dims[Y] = (int)longDims[Y];
	dims[Z] = (int)longDims[Z];
	_bParams->SetGrid(dims);
}

void BarbAppearanceSubtab::yDimChanged(int i) {
	vector<long> longDims = _bParams->GetGrid();
	int dims[3];

	dims[X] = (int)longDims[X];
	dims[Y] = i;
	dims[Z] = (int)longDims[Z];
	_bParams->SetGrid(dims);
}

void BarbAppearanceSubtab::zDimChanged(int i) {
	vector<long> longDims = _bParams->GetGrid();
	int dims[3];

	dims[X] = (int)longDims[X];
	dims[Y] = (int)longDims[Y];
	dims[Z] = i;
	_bParams->SetGrid(dims);
}

void BarbAppearanceSubtab::lengthChanged(double d) {
	_bParams->SetLengthScale(d);
}

void BarbAppearanceSubtab::thicknessChanged(double d) {
	_bParams->SetLineThickness(d);
}

void BarbAppearanceSubtab::recalculateScales() {
	_bParams->SetNeedToRecalculateScales(true);
}
