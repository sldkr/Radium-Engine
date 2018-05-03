#include <Gui/LightCreator.hpp>
#include <QPushButton>
#include <QColorDialog>
#include <QPalette>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QAbstractSpinBox>
#include <math.h>

namespace Ra {
namespace Gui {
LightCreator::LightCreator( QWidget* parent ) : QWidget( parent )    {
    setupUi( this );
    m_color = QColor(255,255,255);
    QPalette p;

    p.setColor(QPalette::Background,m_color);
    m_result_color->setAutoFillBackground(true);
    m_result_color->setPalette(p);
    connect(m_button_color, &QPushButton::clicked,this,&LightCreator::open_dialogColor);

    // Intensity
    connect(m_intensity_slider, &QSlider::valueChanged, this , &LightCreator::slot_intensity_slide_to_spin );
    connect(this,&LightCreator::sig_intensity_slide_to_spin,m_intensity_spinbox,&QDoubleSpinBox::setValue);

    connect(m_intensity_spinbox, static_cast<void (QDoubleSpinBox::*) (double)>(&QDoubleSpinBox::valueChanged), this , &LightCreator::slot_intensity_spin_to_slide );
    connect(this,&LightCreator::sig_intensity_spin_to_slide,m_intensity_slider,&QSlider::setValue);

    // Angle
    connect(m_angle_slider, &QSlider::valueChanged, this , &LightCreator::slot_angle_slide_to_spin );
    connect(this,&LightCreator::sig_angle_slide_to_spin,m_angle_spinbox,&QDoubleSpinBox::setValue);

    connect(m_angle_spinbox, static_cast<void (QDoubleSpinBox::*) (double)>(&QDoubleSpinBox::valueChanged), this , &LightCreator::slot_angle_spin_to_slide );
    connect(this,&LightCreator::sig_angle_spin_to_slide,m_angle_slider,&QSlider::setValue);

    // Falloff

    connect(m_falloff_slider, &QSlider::valueChanged, this , &LightCreator::slot_falloff_slide_to_spin );
    connect(this,&LightCreator::sig_falloff_slide_to_spin,m_falloff_spinbox,&QDoubleSpinBox::setValue);

    connect(m_falloff_spinbox, static_cast<void (QDoubleSpinBox::*) (double)>(&QDoubleSpinBox::valueChanged), this , &LightCreator::slot_falloff_spin_to_slide );
    connect(this,&LightCreator::sig_falloff_spin_to_slide,m_falloff_slider,&QSlider::setValue);



}

void LightCreator::open_dialogColor(){
  m_color = QColorDialog::getColor ();
  QPalette p;

  p.setColor(QPalette::Background,m_color);
  m_result_color->setPalette(p);

}

// Intensity
void LightCreator::slot_intensity_slide_to_spin(int val){
  m_intensity_val = (double) val;
  emit sig_intensity_slide_to_spin(m_intensity_val);
}

void LightCreator::slot_intensity_spin_to_slide(double val){
  m_intensity_val = val;
  int tmp = round(val);
  emit sig_intensity_spin_to_slide(tmp);
}

// Angle

void LightCreator::slot_angle_slide_to_spin(int val){
  m_angle_val = (double) val;
  emit sig_angle_slide_to_spin(m_angle_val);
}

void LightCreator::slot_angle_spin_to_slide(double val){
  m_angle_val = val;
  int tmp = round(val);
  emit sig_angle_spin_to_slide(tmp);
}


// Falloff
void LightCreator::slot_falloff_slide_to_spin(int val){
  m_falloff_val = (double) val;
  emit sig_falloff_slide_to_spin(m_falloff_val);
}

void LightCreator::slot_falloff_spin_to_slide(double val){
  m_falloff_val = val;
  int tmp = round(val);
  emit sig_falloff_spin_to_slide(tmp);
}





} // namespace Gui
} // namespace Ra