/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

void LookAndFeel::drawRotarySlider(juce::Graphics& g,
											   int x,
											   int y,
											   int width,
											   int height,
											   float sliderPosProportional,
											   float rotaryStartAngle,
											   float rotaryEndAngle,
											   juce::Slider& slider)
{
	using namespace juce;

	auto bounds = Rectangle<float>(x, y, width, height);//outer Bounding box

	auto enabled = slider.isEnabled();

	g.setColour(enabled ? Colour(97u, 18u, 167u) : Colours::darkgrey);
	g.fillEllipse(bounds);

	g.setColour(enabled ? Colour(255u, 154u, 1u) : Colours::grey);
	g.drawEllipse(bounds, 1.f);//border of slider

	if (auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider))
	{
		auto center = bounds.getCentre();
		Path p;

		Rectangle<float> r;
		r.setLeft(center.getX() - 2);
		r.setRight(center.getX() + 2);
		r.setTop(bounds.getY());
		r.setBottom(center.getY() - rswl->getTextHeight() * 1.5);

		p.addRoundedRectangle(r, 2.f);

		jassert(rotaryStartAngle < rotaryEndAngle);//Just check if the start angle is less then end angle

		auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);

		p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));

		g.fillPath(p);

		//Text inside of rotter 
		g.setFont(rswl->getTextHeight());
		auto text = rswl->getDisplayString();
		auto strWidth = g.getCurrentFont().getStringWidth(text);

		r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
		r.setCentre(bounds.getCentre());

		g.setColour(enabled ? Colours::black : Colours::darkgrey);
		g.fillRect(r);

		g.setColour(enabled ? Colours::white : Colours::lightgrey);
		g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
	}
}

void LookAndFeel::drawToggleButton(juce::Graphics& g,
								   juce::ToggleButton& toggleButton,
								   bool shouldDrawButtonAsHighlighted,
								   bool shouldDrawButtonAsDown)
{
	using namespace juce;

	if (auto* pb = dynamic_cast<PowerButton*>(&toggleButton))
	{
		Path powerButton;

		auto bounds = toggleButton.getLocalBounds();

		auto size = jmin(bounds.getWidth(), bounds.getHeight()) - 6;
		auto r = bounds.withSizeKeepingCentre(size, size).toFloat();

		float ang = 30.f; //30.f;

		size -= 6;

		powerButton.addCentredArc(r.getCentreX(),
			r.getCentreY(),
			size * 0.5,
			size * 0.5,
			0.f,
			degreesToRadians(ang),
			degreesToRadians(360.f - ang),
			true);

		powerButton.startNewSubPath(r.getCentreX(), r.getY());
		powerButton.lineTo(r.getCentre());

		PathStrokeType pst(2.f, PathStrokeType::JointStyle::curved);

		auto color = toggleButton.getToggleState() ? Colours::dimgrey : Colour(0u, 172u, 1u);

		g.setColour(color);
		g.strokePath(powerButton, pst);
		g.drawEllipse(r, 2);
	}
	else if (auto* analyzerButton = dynamic_cast<AnalyzerButton*>(&toggleButton))
	{
		auto color = !toggleButton.getToggleState() ? Colours::dimgrey : Colour(0u, 172u, 1u);

		g.setColour(color);

		auto bounds = toggleButton.getLocalBounds();
		g.drawRect(bounds);

		g.strokePath(analyzerButton->randomPath, PathStrokeType(1.f));
	}
}

//==============================================================================
void RotarySliderWithLabels::paint(juce::Graphics& graphics)
{
	using namespace juce;

	auto startAng = degreesToRadians(180.f + 45.f); // star point of rotter
	auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi; //end point of rotter

	auto range = getRange();

	auto sliderBounds = getSliderBounds();

	//    g.setColour(Colours::red);
	//    g.drawRect(getLocalBounds());
	//    g.setColour(Colours::yellow);
	//    g.drawRect(sliderBounds);

	getLookAndFeel().drawRotarySlider(graphics,
									  sliderBounds.getX(),
									  sliderBounds.getY(),
									  sliderBounds.getWidth(),
									  sliderBounds.getHeight(),
									  //This is normalize value of our slider
									  jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
									  startAng,endAng,*this);

	auto center = sliderBounds.toFloat().getCentre();//center of slider bounds
	auto radius = sliderBounds.getWidth() * 0.5f;

	graphics.setColour(Colour(0u, 172u, 1u));// Color of text
	graphics.setFont(getTextHeight());

	auto numChoices = labels.size();
	for (int i = 0; i < numChoices; ++i)
	{
		auto pos = labels[i].pos;
		jassert(0.f <= pos);
		jassert(pos <= 1.f);

		auto ang = jmap(pos, 0.f, 1.f, startAng, endAng);

		auto centerPoint = center.getPointOnCircumference(radius + getTextHeight() * 0.5f + 1, ang);

		Rectangle<float> rect;
		auto str = labels[i].label;
		rect.setSize(graphics.getCurrentFont().getStringWidth(str), getTextHeight());
		rect.setCentre(centerPoint);
		rect.setY(rect.getY() + getTextHeight());

		graphics.drawFittedText(str, rect.toNearestInt(), juce::Justification::centred, 1);
	}

}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
	auto bounds = getLocalBounds();

	//make our ellipse to circle 
	auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());

	//as we have text on each side we have to shrink size of circle more 
	size -= getTextHeight() * 2;

	//bounding box of rotter 
	juce::Rectangle<int> r;
	r.setSize(size, size);
	r.setCentre(bounds.getCentreX(), 0);
	r.setY(2);

	return r;
}

juce::String RotarySliderWithLabels::getDisplayString() const
{
	if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
		return choiceParam->getCurrentChoiceName();

	juce::String str;
	bool addK = false;//just check if we are going to add k in the end to convert the 20000 to 20KHz

	if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param))
	{
		float val = getValue();

		if (val > 999.f)
		{
			val /= 1000.f; //1001 / 1000 = 1.001
			addK = true;
		}

		str = juce::String(val, (addK ? 2 : 0));// Default constructor to show only 2 values after the decimal point 
	}
	else
	{
		jassertfalse; //this shouldn't happen!
	}

	if (suffix.isNotEmpty())
	{
		str << " ";
		if (addK)
			str << "k";

		str << suffix;
	}

	return str;
}

//==============================================================================
ResponseCurveComponent::ResponseCurveComponent(AudioPlugin_TestAudioProcessor& p) :	audioProcessor(p),	leftPathProducer(audioProcessor.leftChannelFifo),
	rightPathProducer(audioProcessor.rightChannelFifo)
{
	//Updated as listener 
	const auto& params = audioProcessor.getParameters();
	for (auto param : params)
	{
		param->addListener(this);
	}

	updateChain();

	startTimerHz(60);
}

ResponseCurveComponent::~ResponseCurveComponent()
{
	const auto& params = audioProcessor.getParameters();
	for (auto param : params)
	{
		param->removeListener(this);
	}
}

void ResponseCurveComponent::updateResponseCurve()
{
	using namespace juce;
	auto responseArea = getAnalysisArea();

	auto w = responseArea.getWidth();

	auto& lowcut = monoChain.get<ChainPositions::LowCut>();
	auto& peak = monoChain.get<ChainPositions::Peak>();
	auto& highcut = monoChain.get<ChainPositions::HighCut>();

	auto sampleRate = audioProcessor.getSampleRate();

	std::vector<double> mags;

	mags.resize(w);

	// All the background lines in response curve
	for (int i = 0; i < w; ++i)
	{
		double mag = 1.f;
		auto freq = mapToLog10(double(i) / double(w), 20.0, 20000.0);

		if (!monoChain.isBypassed<ChainPositions::Peak>())
			mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);

		if (!monoChain.isBypassed<ChainPositions::LowCut>())
		{
			if (!lowcut.isBypassed<0>())
				mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
			if (!lowcut.isBypassed<1>())
				mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
			if (!lowcut.isBypassed<2>())
				mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
			if (!lowcut.isBypassed<3>())
				mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
		}

		if (!monoChain.isBypassed<ChainPositions::HighCut>())
		{
			if (!highcut.isBypassed<0>())
				mag *= highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
			if (!highcut.isBypassed<1>())
				mag *= highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
			if (!highcut.isBypassed<2>())
				mag *= highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
			if (!highcut.isBypassed<3>())
				mag *= highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
		}

		mags[i] = Decibels::gainToDecibels(mag);
	}

	responseCurve.clear();

	const double outputMin = responseArea.getBottom();
	const double outputMax = responseArea.getY();
	auto map = [outputMin, outputMax](double input)
	{
		return jmap(input, -24.0, 24.0, outputMin, outputMax);
	};

	responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));

	for (size_t i = 1; i < mags.size(); ++i)
	{
		responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
	}
}

void ResponseCurveComponent::paint(juce::Graphics& g)
{
	using namespace juce;
	// (Our component is opaque, so we must completely fill the background with a solid colour)
	g.fillAll(Colours::black);

	//Background image in response curve 
	drawBackgroundGrid(g);

	auto responseArea = getAnalysisArea();

	if (shouldShowFFTAnalysis)
	{
		auto leftChannelFFTPath = leftPathProducer.getPath();
		leftChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));//Translate our path to analysis bounding box origin

		g.setColour(Colour(97u, 18u, 167u)); //purple-
		g.strokePath(leftChannelFFTPath, PathStrokeType(1.f));//Path is drawn at 0,0

		auto rightChannelFFTPath = rightPathProducer.getPath();
		rightChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));

		g.setColour(Colour(215u, 201u, 134u));
		g.strokePath(rightChannelFFTPath, PathStrokeType(1.f));
	}

	g.setColour(Colours::white);
	g.strokePath(responseCurve, PathStrokeType(2.f));

	Path border;

	border.setUsingNonZeroWinding(false);

	border.addRoundedRectangle(getRenderArea(), 4);
	border.addRectangle(getLocalBounds());

	g.setColour(Colours::black);

	g.fillPath(border);

	drawTextLabels(g);

	g.setColour(Colours::orange);
	g.drawRoundedRectangle(getRenderArea().toFloat(), 4.f, 1.f);
}

// All the frequencies in the background image 
std::vector<float> ResponseCurveComponent::getFrequencies()
{
	return std::vector<float>
	{
		20, /*30, 40,*/ 50, 100,
			200, /*300, 400,*/ 500, 1000,
			2000, /*3000, 4000,*/ 5000, 10000,
			20000
	};
}

std::vector<float> ResponseCurveComponent::getGains()
{
	return std::vector<float>
	{
		-24, -12, 0, 12, 24
	};
}

std::vector<float> ResponseCurveComponent::getXs(const std::vector<float>& freqs, float left, float width)
{
	std::vector<float> xs;
	for (auto f : freqs)
	{
		auto normX = juce::mapFromLog10(f, 20.f, 20000.f);
		xs.push_back(left + width * normX);
	}

	return xs;
}

//Background grid
void ResponseCurveComponent::drawBackgroundGrid(juce::Graphics& g)
{
	using namespace juce;
	auto freqs = getFrequencies();

	auto renderArea = getAnalysisArea();
	auto left = renderArea.getX();
	auto right = renderArea.getRight();
	auto top = renderArea.getY();
	auto bottom = renderArea.getBottom();
	auto width = renderArea.getWidth();

	auto xs = getXs(freqs, left, width);

	g.setColour(Colours::dimgrey);
	for (auto x : xs)
	{
		g.drawVerticalLine(x, top, bottom);//Vertical lines
	}

	auto gain = getGains();

	for (auto gDb : gain)
	{
		auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));

		g.setColour(gDb == 0.f ? Colour(0u, 172u, 1u) : Colours::darkgrey);
		g.drawHorizontalLine(y, left, right); // Horizontal lines
	}
}

void ResponseCurveComponent::drawTextLabels(juce::Graphics& g)
{
	using namespace juce;
	g.setColour(Colours::lightgrey);
	const int fontHeight = 10;
	g.setFont(fontHeight);

	auto renderArea = getAnalysisArea();
	auto left = renderArea.getX();

	auto top = renderArea.getY();
	auto bottom = renderArea.getBottom();
	auto width = renderArea.getWidth();

	// Freqs labels
	auto freqs = getFrequencies();
	auto xs = getXs(freqs, left, width);

	for (int i = 0; i < freqs.size(); ++i)
	{
		auto f = freqs[i];
		auto x = xs[i];

		bool addK = false;
		String str;
		if (f > 999.f)
		{
			addK = true;
			f /= 1000.f;
		}

		str << f;
		if (addK)
			str << "k";
		str << "Hz";

		auto textWidth = g.getCurrentFont().getStringWidth(str);

		Rectangle<int> r;

		r.setSize(textWidth, fontHeight);
		r.setCentre(x, 0);
		r.setY(1);

		g.drawFittedText(str, r, juce::Justification::centred, 1);
	}

	//Gain labels
	auto gain = getGains();

	for (auto gDb : gain)
	{
		auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));

		String str;
		if (gDb > 0)
			str << "+";
		str << gDb;

		auto textWidth = g.getCurrentFont().getStringWidth(str);

		Rectangle<int> r;
		r.setSize(textWidth, fontHeight);
		r.setX(getWidth() - textWidth);
		r.setCentre(r.getCentreX(), y);

		g.setColour(gDb == 0.f ? Colour(0u, 172u, 1u) : Colours::lightgrey);

		//Right side text
		g.drawFittedText(str, r, juce::Justification::centredLeft, 1);

		str.clear();
		str << (gDb - 24.f);

		r.setX(1);
		textWidth = g.getCurrentFont().getStringWidth(str);
		r.setSize(textWidth, fontHeight);
		g.setColour(Colours::lightgrey);

		//Left side text
		g.drawFittedText(str, r, juce::Justification::centredLeft, 1);
	}
}

void ResponseCurveComponent::resized()
{
	using namespace juce;

	responseCurve.preallocateSpace(getWidth() * 3);
	updateResponseCurve();
}

//Whenever parameter set this value 
void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue)
{
	parametersChanged.set(true);
}

//
void ResponseCurveComponent::timerCallback()
{
	if (shouldShowFFTAnalysis)
	{
		auto fftBounds = getAnalysisArea().toFloat();
		auto sampleRate = audioProcessor.getSampleRate();

		leftPathProducer.process(fftBounds, sampleRate);
		rightPathProducer.process(fftBounds, sampleRate);
	}

	if (parametersChanged.compareAndSetBool(false, true))
	{
		updateChain(); // Instance change in cover as parameter change
		updateResponseCurve();
	}

	repaint();
}

void PathProducer::process(juce::Rectangle<float> fftBounds, double sampleRate)
{
	juce::AudioBuffer<float> tempIncomingBuffer;

	//While there are buffers to pull from Fifo, if u can pull then send it to FFT
	while (leftChannelFifo->getNumCompleteBuffersAvailable() > 0)
	{
		if (leftChannelFifo->getAudioBuffer(tempIncomingBuffer))
		{
			//Shift data forward 
			auto size = tempIncomingBuffer.getNumSamples();

			juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, 0), //Copy the buffer to zero index 
											  monoBuffer.getReadPointer(0, size),//
											  monoBuffer.getNumSamples() - size);

			juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, monoBuffer.getNumSamples() - size),//Copy new incoming data to 0th position
												tempIncomingBuffer.getReadPointer(0, 0),
												size);

			//Send mono buffer to FFT
			leftChannelFFTDataGenerator.produceFFTDataForRendering(monoBuffer, -48.f);

		}
	}

	const auto fftSize = leftChannelFFTDataGenerator.getFFTSize();

	//4800/2048 = 23Hz ,_ this is the bin width
	const auto binWidth = sampleRate / double(fftSize);

	//if there are FFT data buffers to pull 
	while (leftChannelFFTDataGenerator.getNumAvailableFFTDataBlocks() > 0)
	{
		//if we can pull a buffer
		std::vector<float> fftData;
		if (leftChannelFFTDataGenerator.getFFTData(fftData))
		{
			//generate path
			pathProducer.generatePath(fftData, fftBounds, fftSize, binWidth, -48.f);
		}
	}

	/*while there are paths that can be pulled
		pull as many as we can
		display the most recent path
	*/
	while (pathProducer.getNumPathsAvailable() > 0)
	{
		pathProducer.getPath(leftChannelFFTPath);
	}
}

void ResponseCurveComponent::updateChain()
{
	auto chainSettings = getChainSettings(audioProcessor.apvts);

	monoChain.setBypassed<ChainPositions::LowCut>(chainSettings.lowCutBypassed);
	monoChain.setBypassed<ChainPositions::Peak>(chainSettings.peakBypassed);
	monoChain.setBypassed<ChainPositions::HighCut>(chainSettings.highCutBypassed);

	auto peakCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate());
	updateCoefficients(monoChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);

	auto lowCutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
	auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());

	updateCutFilter(monoChain.get<ChainPositions::LowCut>(),
		lowCutCoefficients,
		chainSettings.lowCutSlope);

	updateCutFilter(monoChain.get<ChainPositions::HighCut>(),
		highCutCoefficients,
		chainSettings.highCutSlope);
}

//Get the area where we are drawing the curve  
juce::Rectangle<int> ResponseCurveComponent::getRenderArea()
{
	// reduce the render area to write some values 
	auto bounds = getLocalBounds();

	//bounds.reduce(JUCE_LIVE_CONSTANT(5), JUCE_LIVE_CONSTANT(5));
	bounds.removeFromTop(12);
	bounds.removeFromBottom(2);
	bounds.removeFromLeft(20);
	bounds.removeFromRight(20);

	return bounds;
}

juce::Rectangle<int> ResponseCurveComponent::getAnalysisArea()
{
	auto bounds = getRenderArea();
	bounds.removeFromTop(4);
	bounds.removeFromBottom(4);
	return bounds;
}

//==============================================================================
	AudioPlugin_TestAudioProcessorEditor::AudioPlugin_TestAudioProcessorEditor(AudioPlugin_TestAudioProcessor& p)
		: AudioProcessorEditor(&p), audioProcessor(p),
		//Initialize all the sliders
		peakFreqSlider(*audioProcessor.apvts.getParameter("Peak Freq"), "Hz"),
		peakGainSlider(*audioProcessor.apvts.getParameter("Peak Gain"), "dB"),
		peakQualitySlider(*audioProcessor.apvts.getParameter("Peak Quality"), ""),
		lowCutFreqSlider(*audioProcessor.apvts.getParameter("LowCut Freq"), "Hz"),
		highCutFreqSlider(*audioProcessor.apvts.getParameter("HighCut Freq"), "Hz"),
		lowCutSlopeSlider(*audioProcessor.apvts.getParameter("LowCut Slope"), "dB/Oct"),
		highCutSlopeSlider(*audioProcessor.apvts.getParameter("HighCut Slope"), "db/Oct"),

		responseCurveComponent(audioProcessor),

		peakFreqSliderAttachment(audioProcessor.apvts, "Peak Freq", peakFreqSlider),
		peakGainSliderAttachment(audioProcessor.apvts, "Peak Gain", peakGainSlider),
		peakQualitySliderAttachment(audioProcessor.apvts, "Peak Quality", peakQualitySlider),
		lowCutFreqSliderAttachment(audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
		highCutFreqSliderAttachment(audioProcessor.apvts, "HighCut Freq", highCutFreqSlider),
		lowCutSlopeSliderAttachment(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSlider),
		highCutSlopeSliderAttachment(audioProcessor.apvts, "HighCut Slope", highCutSlopeSlider),

		lowcutBypassButtonAttachment(audioProcessor.apvts, "LowCut Bypassed", lowcutBypassButton),
		peakBypassButtonAttachment(audioProcessor.apvts, "Peak Bypassed", peakBypassButton),
		highcutBypassButtonAttachment(audioProcessor.apvts, "HighCut Bypassed", highcutBypassButton),
		analyzerEnabledButtonAttachment(audioProcessor.apvts, "Analyzer Enabled", analyzerEnabledButton)
	{
		peakFreqSlider.labels.add({ 0.f, "20Hz" });
		peakFreqSlider.labels.add({ 1.f, "20kHz" });

		peakGainSlider.labels.add({ 0.f, "-24dB" });
		peakGainSlider.labels.add({ 1.f, "+24dB" });

		peakQualitySlider.labels.add({ 0.f, "0.1" });
		peakQualitySlider.labels.add({ 1.f, "10.0" });

		lowCutFreqSlider.labels.add({ 0.f, "20Hz" });
		lowCutFreqSlider.labels.add({ 1.f, "20kHz" });

		highCutFreqSlider.labels.add({ 0.f, "20Hz" });
		highCutFreqSlider.labels.add({ 1.f, "20kHz" });

		lowCutSlopeSlider.labels.add({ 0.0f, "12" });
		lowCutSlopeSlider.labels.add({ 1.f, "48" });

		highCutSlopeSlider.labels.add({ 0.0f, "12" });
		highCutSlopeSlider.labels.add({ 1.f, "48" });

		for (auto* comp : getComps())
		{
			addAndMakeVisible(comp);
		}

		peakBypassButton.setLookAndFeel(&lnf);
		highcutBypassButton.setLookAndFeel(&lnf);
		lowcutBypassButton.setLookAndFeel(&lnf);

		analyzerEnabledButton.setLookAndFeel(&lnf);

		auto safePtr = juce::Component::SafePointer<AudioPlugin_TestAudioProcessorEditor>(this);
		peakBypassButton.onClick = [safePtr]()
		{
			if (auto* comp = safePtr.getComponent())
			{
				auto bypassed = comp->peakBypassButton.getToggleState();

				comp->peakFreqSlider.setEnabled(!bypassed);
				comp->peakGainSlider.setEnabled(!bypassed);
				comp->peakQualitySlider.setEnabled(!bypassed);
			}
		};


		lowcutBypassButton.onClick = [safePtr]()
		{
			if (auto* comp = safePtr.getComponent())
			{
				auto bypassed = comp->lowcutBypassButton.getToggleState();

				comp->lowCutFreqSlider.setEnabled(!bypassed);
				comp->lowCutSlopeSlider.setEnabled(!bypassed);
			}
		};

		highcutBypassButton.onClick = [safePtr]()
		{
			if (auto* comp = safePtr.getComponent())
			{
				auto bypassed = comp->highcutBypassButton.getToggleState();

				comp->highCutFreqSlider.setEnabled(!bypassed);
				comp->highCutSlopeSlider.setEnabled(!bypassed);
			}
		};

		analyzerEnabledButton.onClick = [safePtr]()
		{
			if (auto* comp = safePtr.getComponent())
			{
				auto enabled = comp->analyzerEnabledButton.getToggleState();
				comp->responseCurveComponent.toggleAnalysisEnablement(enabled);
			}
		};

		setSize(550, 500);
	}

AudioPlugin_TestAudioProcessorEditor::~AudioPlugin_TestAudioProcessorEditor()
{
   /* const auto& params = audioProcessor.getParameters();
    for (auto param : params)
    {
        param->removeListener(this);
    }*/
	peakBypassButton.setLookAndFeel(nullptr);
	highcutBypassButton.setLookAndFeel(nullptr);
	lowcutBypassButton.setLookAndFeel(nullptr);

	analyzerEnabledButton.setLookAndFeel(nullptr);
}

//==============================================================================
void AudioPlugin_TestAudioProcessorEditor::paint (juce::Graphics& g)
{
     using namespace juce;
    
    //g.fillAll (Colours::black);
    g.fillAll (Colours::darkblue);
    
    Path curve;
    
    auto bounds = getLocalBounds();
    auto center = bounds.getCentre();
    
    g.setFont(Font("Iosevka Term Slab", 30, 0)); //https://github.com/be5invis/Iosevka
    
    String title { "AudioPlugin_Test" };
    g.setFont(30);
    auto titleWidth = g.getCurrentFont().getStringWidth(title);
    
    curve.startNewSubPath(center.x, 32);
    curve.lineTo(center.x - titleWidth * 0.45f, 32);
    
    auto cornerSize = 20;
    auto curvePos = curve.getCurrentPosition();
    curve.quadraticTo(curvePos.getX() - cornerSize, curvePos.getY(),
                      curvePos.getX() - cornerSize, curvePos.getY() - 16);
    curvePos = curve.getCurrentPosition();
    curve.quadraticTo(curvePos.getX(), 2,
                      curvePos.getX() - cornerSize, 2);
    
    curve.lineTo({0.f, 2.f});
    curve.lineTo(0.f, 0.f);
    curve.lineTo(center.x, 0.f);
    curve.closeSubPath();
    
    g.setColour(Colour(97u, 18u, 167u));
    g.fillPath(curve);
    
    curve.applyTransform(AffineTransform().scaled(-1, 1));
    curve.applyTransform(AffineTransform().translated(getWidth(), 0));
    g.fillPath(curve);
    
    
    g.setColour(Colour(255u, 154u, 1u));
    g.drawFittedText(title, bounds, juce::Justification::centredTop, 1);
    
    g.setColour(Colours::white);
    g.setFont(14);
    g.drawFittedText("LowCut", lowCutSlopeSlider.getBounds(), juce::Justification::centredBottom, 1);
    g.drawFittedText("Peak", peakQualitySlider.getBounds(), juce::Justification::centredBottom, 1);
    g.drawFittedText("HighCut", highCutSlopeSlider.getBounds(), juce::Justification::centredBottom, 1);
    
    auto buildDate = Time::getCompilationDate().toString(true, false);
    auto buildTime = Time::getCompilationDate().toString(false, true);
    g.setFont(12);
    g.drawFittedText("Build: " + buildDate + "\n" + buildTime, highCutSlopeSlider.getBounds().withY(6), Justification::topRight, 2);
}

void AudioPlugin_TestAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds = getLocalBounds();//bounding box for the component
	bounds.removeFromTop(4);

	auto analyzerEnabledArea = bounds.removeFromTop(25);

	analyzerEnabledArea.setWidth(50);
	analyzerEnabledArea.setX(5);
	analyzerEnabledArea.removeFromTop(2);

	analyzerEnabledButton.setBounds(analyzerEnabledArea);

	bounds.removeFromTop(5);// Space between the response curve and sliders

	float hRatio = 25.f / 100.f; //JUCE_LIVE_CONSTANT(25) / 100.f;
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);//reserve 33% of height (Top portion)

	responseCurveComponent.setBounds(responseArea);
	bounds.removeFromTop(5);

    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);//reserve 33% area of the remaining area(left area)
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.5);//reserve 33% area of the remaining area(right area)

	lowcutBypassButton.setBounds(lowCutArea.removeFromTop(25));//bypass button
    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight()*0.5));//remove half of rectangle 
    lowCutSlopeSlider.setBounds(lowCutArea);

	highcutBypassButton.setBounds(highCutArea.removeFromTop(25));//bypass button
    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 0.5));//remove half of rectangle 
    highCutSlopeSlider.setBounds(highCutArea);

	peakBypassButton.setBounds(bounds.removeFromTop(25));//bypass button
    peakFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.33));//In the middle , 1st one 
    peakGainSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5));//In the middle , 2nd one 
    peakQualitySlider.setBounds(bounds);//In the middle , 3rd one 
}

std::vector<juce::Component*> AudioPlugin_TestAudioProcessorEditor::getComps()
{
	return { &peakFreqSlider,
			& peakGainSlider,
			& peakQualitySlider,
			& lowCutFreqSlider,
			& highCutFreqSlider,
			& lowCutSlopeSlider,
			& highCutSlopeSlider,
			& responseCurveComponent,

			& lowcutBypassButton,
			& peakBypassButton,
			& highcutBypassButton,
			& analyzerEnabledButton
    };
}
