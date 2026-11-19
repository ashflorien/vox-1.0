#include "PluginEditor.h"
#include <cmath>

namespace {
constexpr float W = 1200.f, H = 760.f;
void text(juce::Graphics& g, const juce::String& s, juce::Rectangle<float> r,
          float size, juce::Colour colour, juce::Justification align = juce::Justification::centred,
          bool bold = false) {
    g.setColour(colour);
    g.setFont(juce::Font(juce::FontOptions(size, bold ? juce::Font::bold : juce::Font::plain)));
    g.drawFittedText(s, r.toNearestInt(), align, 1);
}
void led(juce::Graphics& g, float x, float y, juce::Colour colour) {
    g.setColour(juce::Colours::black); g.fillRoundedRectangle(x-2,y-2,23,23,3);
    g.setColour(colour.withAlpha(.2f)); g.fillEllipse(x-6,y-6,30,30);
    g.setColour(colour); g.fillRoundedRectangle(x+2,y+2,15,15,2);
    g.setColour(juce::Colours::white.withAlpha(.4f)); g.fillRoundedRectangle(x+4,y+3,5,4,1);
}
void readout(juce::Graphics& g, juce::Rectangle<float> r, const juce::String& value, juce::Colour ink) {
    g.setColour(juce::Colours::black.withAlpha(.76f)); g.fillRoundedRectangle(r, 7);
    g.setColour(ink.withAlpha(.75f)); g.drawRoundedRectangle(r,7,1.5f);
    text(g,value,r.reduced(4),23,ink);
}
}

void VoxOneAudioProcessorEditor::HardwareLookAndFeel::drawLinearSlider(
    juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
    float, float, const juce::Slider::SliderStyle, juce::Slider& slider) {
    const float cx = x + width * .5f;
    const auto groove = juce::Rectangle<float>(cx - 13.f, y+7.f, 26.f, height-14.f);
    g.setColour(juce::Colours::black); g.fillRoundedRectangle(groove, 13.f);
    g.setColour(juce::Colours::white.withAlpha(.12f)); g.drawRoundedRectangle(groove, 13.f, 1.f);
    const bool compression = slider.getComponentID() == "comp";
    const auto active = compression ? juce::Colour(0xffffb228) : juce::Colour(0xff7bf34b);
    const int count = 25;
    for (int i=0; i<count; ++i) {
        const float fraction = (i+.5f)/count;
        const float yy = groove.getBottom()-12.f-fraction*(groove.getHeight()-24.f);
        const float norm = (float)slider.valueToProportionOfLength(slider.getValue());
        g.setColour(fraction <= norm ? active.interpolatedWith(juce::Colours::yellow, compression ? .25f : .0f)
                                      : active.withAlpha(.08f));
        g.fillRoundedRectangle(cx-8.f,yy-3.f,16.f,5.f,1.2f);
    }
    if (compression) return; // Compression is represented by a luminous reduction bar.
    const auto cap = juce::Rectangle<float>(cx-24.f,sliderPos-19.f,48.f,38.f);
    g.setColour(juce::Colours::black.withAlpha(.65f)); g.fillRoundedRectangle(cap.translated(2,4),4.f);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xfff7f3ec),cap.getX(),cap.getY(),
                                           juce::Colour(0xff88898c),cap.getX(),cap.getBottom(),false));
    g.fillRoundedRectangle(cap,4.f);
    g.setColour(juce::Colour(0xff37383a)); g.drawRoundedRectangle(cap,4.f,1.5f);
    for (int i=0;i<3;++i) { float yy=cap.getY()+10.f+i*8.f;
        g.setColour(juce::Colours::white.withAlpha(.65f)); g.drawLine(cap.getX()+4,yy,cap.getRight()-4,yy,1.f);
        g.setColour(juce::Colours::black.withAlpha(.45f)); g.drawLine(cap.getX()+4,yy+2,cap.getRight()-4,yy+2,1.f);
    }
}

void VoxOneAudioProcessorEditor::HardwareLookAndFeel::drawRotarySlider(
    juce::Graphics& g, int x, int y, int width, int height, float position,
    float start, float end, juce::Slider&) {
    const auto c = juce::Point<float>(x+width*.5f,y+height*.5f);
    const float r = juce::jmin(width,height)*.38f;
    for(int i=0;i<=12;++i) {
        float a=start+(end-start)*i/12.f;
        auto p=c.getPointOnCircumference(r+13.f,a);
        g.setColour(ink.withAlpha(.8f)); g.fillEllipse(p.x-2.5f,p.y-2.5f,5.f,5.f);
    }
    g.setColour(juce::Colours::black.withAlpha(.7f)); g.fillEllipse(c.x-r-3,c.y-r+5,2*r+6,2*r+6);
    g.setColour(juce::Colour(0xff55575a)); g.fillEllipse(c.x-r-4,c.y-r-4,2*r+8,2*r+8);
    g.setColour(accent.darker(.35f)); g.fillEllipse(c.x-r,c.y-r,2*r,2*r);
    g.setColour(accent); g.fillEllipse(c.x-r+6,c.y-r+6,2*r-12,2*r-12);
    const float angle=start+position*(end-start);
    const auto tip=c.getPointOnCircumference(r*.75f,angle);
    g.setColour(juce::Colours::black.withAlpha(.8f)); g.drawLine(c.x+2,c.y+2,tip.x+2,tip.y+2,5.f);
    g.setColour(juce::Colours::white); g.drawLine(c.x,c.y,tip.x,tip.y,4.f);
}

VoxOneAudioProcessorEditor::VoxOneAudioProcessorEditor(VoxOneAudioProcessor& p)
: AudioProcessorEditor(&p), processor(p) {
    setLookAndFeel(&look);
    auto fader=[this](juce::Slider& s, const juce::String& id) {
        s.setComponentID(id);
        s.setSliderStyle(juce::Slider::LinearVertical);
        s.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
        addAndMakeVisible(s);
    };
    fader(gate,"gate"); fader(comp,"comp"); fader(gain,"gain");
    saturation.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    saturation.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
    addAndMakeVisible(saturation);
    gateAttachment=std::make_unique<SA>(processor.parameters,"gate",gate);
    compAttachment=std::make_unique<SA>(processor.parameters,"comp",comp);
    gainAttachment=std::make_unique<SA>(processor.parameters,"gain",gain);
    satAttachment=std::make_unique<SA>(processor.parameters,"saturation",saturation);
    skin.addItemList({"Midnight","Miami Vice","Classic Vox","Pastel","SSL 4000"},1);
    satType.addItemList({"Off","Tape","Tube","Clip","Warm"},1);
    meterMode.addItemList({"Input","Output","Gain Reduction"},1);
    meterMode.setSelectedId(2,juce::dontSendNotification);
    skinAttachment=std::make_unique<CA>(processor.parameters,"skin",skin);
    typeAttachment=std::make_unique<CA>(processor.parameters,"satType",satType);
    for(auto* c : {&skin,&satType,&meterMode}) addAndMakeVisible(*c);
    power.setButtonText(juce::String::fromUTF8("⏻"));
    power.setClickingTogglesState(true);
    power.setToggleState(true,juce::dontSendNotification);
    // Local bypass; not yet exposed as a host-automatable parameter.
    power.onClick=[this] { processor.uiBypassed.store(!power.getToggleState()); repaint(); };
    addAndMakeVisible(power);
    setSize(1200,760);
    setResizable(true,true);
    setResizeLimits(840,532,1800,1140);
    getConstrainer()->setFixedAspectRatio(W/H);
    applyTheme();
    startTimerHz(30);
}
VoxOneAudioProcessorEditor::~VoxOneAudioProcessorEditor() { setLookAndFeel(nullptr); }

juce::Rectangle<float> VoxOneAudioProcessorEditor::viewport() const {
    const float s=juce::jmin(getWidth()/W,getHeight()/H);
    return { (getWidth()-W*s)*.5f,(getHeight()-H*s)*.5f,W*s,H*s };
}
void VoxOneAudioProcessorEditor::applyTheme() {
    const int id=juce::jlimit(0,4,(int)std::round(processor.parameters.getRawParameterValue("skin")->load()));
    if(id==currentSkin) return;
    currentSkin=id;
    switch(id) {
        case 0: bg=0xff111820; panel=0xff25303a; ink=0xffe8f3f5; muted=0xffa5b9c3; accent=0xff57d8c8; break;
        case 1: bg=0xff1a1327; panel=0xff30233e; ink=0xffffeafa; muted=0xffc6a6da; accent=0xffff4db2; break;
        case 2: bg=0xff353a3b; panel=0xff54585a; ink=0xfff8eee0; muted=0xffd5c7b4; accent=0xffd4a45a; break;
        case 3: bg=0xffeae0ed; panel=0xffd9cce3; ink=0xff443a55; muted=0xff746681; accent=0xffae81c5; break;
        default: bg=0xff202125; panel=0xff303136; ink=0xfff2eee8; muted=0xffbbb9b7; accent=0xffe23d37; break;
    }
    look.accent=accent;look.ink=ink;look.panel=panel;
    for(auto* c : {&skin,&satType,&meterMode}) {
        c->setColour(juce::ComboBox::backgroundColourId,juce::Colour(0xff141416));
        c->setColour(juce::ComboBox::textColourId,juce::Colours::white);
        c->setColour(juce::ComboBox::outlineColourId,muted.withAlpha(.7f));
        c->setColour(juce::ComboBox::arrowColourId,juce::Colours::white);
    }
    power.setColour(juce::TextButton::buttonColourId,juce::Colour(0xff101512));
    power.setColour(juce::TextButton::buttonOnColourId,juce::Colour(0xff173a22));
    power.setColour(juce::TextButton::textColourOnId,juce::Colour(0xff8cff76));
    power.setColour(juce::TextButton::textColourOffId,muted);
    resized(); repaint();
}
void VoxOneAudioProcessorEditor::resized() {
    auto v=viewport(); const float s=v.getWidth()/W;
    auto place=[&](juce::Component& c,float x,float y,float w,float h) {
        c.setBounds(juce::Rectangle<float>(v.getX()+x*s,v.getY()+y*s,w*s,h*s).toNearestInt());
    };
    place(gate,147,237,100,332); place(comp,400,237,100,332); place(gain,648,237,100,332);
    place(saturation,944,451,168,154);
    place(skin,812,29,285,43); place(power,1122,19,54,54);
    place(meterMode,864,166,245,38); place(satType,864,424,245,37);
}
void VoxOneAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff101114));
    auto v=viewport();const float s=v.getWidth()/W;
    g.saveState();g.addTransform(juce::AffineTransform::scale(s).translated(v.getX(),v.getY()));
    g.setColour(bg);g.fillRoundedRectangle(2,2,W-4,H-4,11);
    g.setColour(ink.withAlpha(.35f));g.drawRoundedRectangle(3,3,W-6,H-6,11,1);
    g.setColour(panel);g.fillRect(4.f,82.f,1192.f,592.f);
    g.setColour(juce::Colours::black.withAlpha(.24f));g.fillRect(4.f,82.f,1192.f,2.f);
    for(float x : {283.f,535.f,786.f}) {g.setColour(ink.withAlpha(.55f));g.drawLine(x,83,x,673,1.3f);g.setColour(juce::Colours::black.withAlpha(.7f));g.drawLine(x+3,83,x+3,673,2.f);}
    g.setColour(ink.withAlpha(.5f));g.drawLine(4,82,1196,82,1.4f);g.drawLine(4,674,1196,674,1.4f);
    text(g,"Vox",{24,15,125,55},43,ink,juce::Justification::centredRight,true);
    text(g,"One",{144,15,116,55},43,currentSkin==4 ? juce::Colour(0xffef342b) : accent,juce::Justification::centredLeft,true);
    g.setColour(ink.withAlpha(.65f));g.drawLine(288,19,288,65,1.5f);
    text(g,"V O C A L   D Y N A M I C S",{315,21,470,48},20,ink);
    text(g,"Skin:",{758,30,55,39},17,muted);
    for(const auto& section : std::initializer_list<std::pair<const char*,float>>{{"GATE",32.f},{"COMP",286.f},{"GAIN",538.f}}) {
        text(g,section.first,{section.second+22,109,196,48},28,ink);
        g.setColour(ink.withAlpha(.6f));g.drawLine(section.second+38,163,section.second+218,163,1.f);
    }
    led(g,241,122,juce::Colour(0xff62fb40));led(g,493,122,juce::Colour(0xffffac20));led(g,744,122,juce::Colour(0xff62fb40));
    auto ticks=[&](float x,const std::vector<std::pair<int,float>>& labels) {
        for(auto [value,yy] : labels) {
            text(g,juce::String(value),{x-63,yy-15,51,30},19,ink,juce::Justification::centredRight);
            g.setColour(ink.withAlpha(.8f));g.drawLine(x-7,yy,x+10,yy,1.6f);
        }
    };
    ticks(151,{{0,250},{-10,308},{-20,366},{-30,424},{-40,482},{-50,540},{-60,598}});
    ticks(404,{{0,250},{-3,300},{-6,350},{-9,400},{-12,450},{-18,525},{-24,598}});
    ticks(650,{{12,250},{6,327},{0,405},{-6,500},{-12,598}});
    readout(g,{73,620,167,44},juce::String(gate.getValue(),1)+" dB",ink);
    readout(g,{575,620,167,44},(gain.getValue()>=0?"+":"")+juce::String(gain.getValue(),1)+" dB",ink);
    readout(g,{409,574,92,37},"-"+juce::String(displayedReduction,1)+" dB",ink);
    text(g,"VU METER",{817,98,337,48},27,ink);
    // Backlit analogue VU instrument, including calibrated scale and needle.
    const juce::Rectangle<float> vu(824,221,338,170);
    g.setColour(juce::Colours::black);g.fillRoundedRectangle(vu.expanded(9),10);
    g.setColour(juce::Colour(0xffd7a65e));g.fillRoundedRectangle(vu,7);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xffffe4a4),vu.getCentreX(),vu.getY(),
                                          juce::Colour(0xffb97a42),vu.getCentreX(),vu.getBottom(),false));
    g.fillRoundedRectangle(vu.reduced(5),5);
    const auto pivot=juce::Point<float>(993.f,401.f);
    const float radius=140.f;
    const int values[]={-20,-10,-7,-5,-3,-1,0,1,2,3};
    for(int i=0;i<10;++i) {
        const float a=juce::degreesToRadians(-62.f+124.f*i/9.f);
        const auto p1=pivot.getPointOnCircumference(radius-18.f,a);
        const auto p2=pivot.getPointOnCircumference(radius-5.f,a);
        g.setColour(i>6?juce::Colour(0xffae2626):juce::Colour(0xff272321));
        g.drawLine(p1.x,p1.y,p2.x,p2.y,2.f);
        auto label=pivot.getPointOnCircumference(radius+12.f,a);
        text(g,(values[i]>0?"+":"")+juce::String(values[i]),{label.x-17,label.y-11,34,22},15,i>6?juce::Colour(0xffb22424):juce::Colour(0xff272321));
    }
    text(g,"VU",{946,321,95,32},24,juce::Colour(0xff443421),juce::Justification::centred,true);
    const int mode=meterMode.getSelectedId();
    const float norm=mode==3?juce::jlimit(0.f,1.f,displayedReduction/24.f):juce::jlimit(0.f,1.f,(displayedMeter+42.f)/45.f);
    const float a=juce::degreesToRadians(-62.f+124.f*norm);
    const auto needle=pivot.getPointOnCircumference(radius-5.f,a);
    g.setColour(juce::Colour(0xff332a24));g.drawLine(pivot.x,pivot.y,needle.x,needle.y,2.5f);
    g.setColour(juce::Colour(0xff292725));g.fillEllipse(pivot.x-5,pivot.y-5,10,10);
    g.setColour(ink.withAlpha(.55f));g.drawRoundedRectangle(vu.expanded(9),10,2.f);
    g.setColour(ink.withAlpha(.55f));g.drawLine(813,410,1168,410,1.2f);
    text(g,"SATURATION",{823,417,321,45},26,ink);
    led(g,1118,427,juce::Colour(0xffff3333));
    text(g,"0 %",{875,598,78,28},20,ink);
    text(g,"100 %",{1101,598,76,28},20,ink);
    readout(g,{943,620,169,43},juce::String(saturation.getValue(),0)+" %",ink);
    text(g,"V O C A L   C O M P R E S S O R   /   E X P A N D E R",{32,695,755,35},17,ink,juce::Justification::centredLeft);
    text(g,currentSkin==4?"SSL 4000":"VoxOne  v0.5",{894,690,267,45},30,ink,juce::Justification::centredRight,true);
    g.restoreState();
}
void VoxOneAudioProcessorEditor::timerCallback() {
    applyTheme();
    const int mode=meterMode.getSelectedId();
    const float target=mode==1?processor.inputLevelDb.load():processor.outputLevelDb.load();
    displayedMeter+=(juce::jlimit(-60.f,6.f,target)-displayedMeter)*.18f;
    displayedReduction+=(processor.gainReductionDb.load()-displayedReduction)*.18f;
    repaint();
}
juce::AudioProcessorEditor* VoxOneAudioProcessor::createEditor() { return new VoxOneAudioProcessorEditor(*this); }
