#define _USE_MATH_DEFINES

#include "ExperimentCalcHelper.h"
#include "Log.h"  //debugging only
#include <qdebug.h>  //debugging only
#include <iostream> //debugging only
#include <fstream> //debugging only

/* DC methods */

void ExperimentCalcHelperClass::GetSamplingParams_staticDAC(HardwareModel_t HWversion, ExperimentNode_t * pNode, const double targetSamplingInterval, double MaxUpdateInterval)
{
    pNode->DCsamplingParams.isFastSlewRate = false;
	int dt_min = 50000;
  int ADCbufsize = 256;
  int DACbufsize = 256;
	switch (HWversion)
	{
	case PRIME:
	case EDGE:
	case PICO:
	case SOLO:
  case PLUS:
		dt_min = SQUIDSTAT_TEENSY_MIN_ADCDC_TIMER_PERIOD;
    ADCbufsize = ADCdcBUF_SIZE_TEENSY;
    DACbufsize = DACdcBUF_SIZE_TEENSY;
		break;
	case PLUS_2_0:
	case SOLO_2_0:
	case PRIME_2_0:
		dt_min = SQUIDSTAT_PIC_MIN_ADCDC_TIMER_PERIOD;
    ADCbufsize = ADCdcBUF_SIZE_PIC;
    DACbufsize = DACdcBUF_SIZE_PIC;
		break;
	default:
		break;
	}

  pNode->DCsamplingParams.isFastSlewRate = false;
	pNode->DCsamplingParams.ADCTimerDiv = 0;
	pNode->DCsamplingParams.ADCBufferSizeEven = pNode->DCsamplingParams.ADCBufferSizeOdd = 1;

  /* If targetSamplingInterval > 1 second, increase Filtersize */
  double transmitInterval;
  uint32_t FilterSize = 0;
  do
  {
    FilterSize++;
    transmitInterval = targetSamplingInterval / FilterSize * SECONDS;
  } while (transmitInterval > MaxUpdateInterval * SECONDS);
  pNode->DCsamplingParams.PointsSkippedPC = FilterSize;

	/* Minimize dt, maximize ADCBufferSizeEven and Odd */
	uint64_t dt;
  pNode->DCsamplingParams.ADCBufferSizeEven = 0;
  do
  {
    pNode->DCsamplingParams.ADCBufferSizeEven++;
    dt = (uint64_t)MAX(dt_min, transmitInterval / pNode->DCsamplingParams.ADCBufferSizeEven);
  } while (dt / dt_min > 1 && pNode->DCsamplingParams.ADCBufferSizeEven < ADCbufsize);
  pNode->DCsamplingParams.ADCBufferSizeOdd = pNode->DCsamplingParams.ADCBufferSizeEven;
  pNode->DCsamplingParams.ADCTimerPeriod = (uint32_t)dt;
  pNode->DCsamplingParams.ADCTimerDiv = 0;
}

void ExperimentCalcHelperClass::GetSamplingParams_potSweep(HardwareModel_t HWversion, const cal_t * calData, ExperimentNode_t * pNode,
    double dEdt, double targetSamplingInterval, double MaxUpdateInterval)
{
    if (dEdt == 0)
    {
        GetSamplingParams_staticDAC(HWversion, pNode, targetSamplingInterval);
        return;
    }
    if (targetSamplingInterval == 0)
    {
        targetSamplingInterval = dEdt / 1000 * calData->m_DACdcP_V * SECONDS;
    }

    int ADCdt_min, DACdt_min, ADCbufsizeMax, DACbufsizeMax;
    switch (HWversion)
    {
    case PRIME:
    case PICO:
    case EDGE:
    case SOLO:
    case PLUS:
        ADCdt_min = SQUIDSTAT_TEENSY_MIN_ADCDC_TIMER_PERIOD;
        DACdt_min = SQUIDSTAT_TEENSY_MIN_DACDC_TIMER_PERIOD;
        ADCbufsizeMax = ADCdcBUF_SIZE_TEENSY;
        DACbufsizeMax = DACdcBUF_SIZE_TEENSY;
        break;
    case PLUS_2_0:
    case PRIME_2_0:
    case SOLO_2_0:
        ADCdt_min = SQUIDSTAT_PIC_MIN_ADCDC_TIMER_PERIOD;
        DACdt_min = SQUIDSTAT_PIC_MIN_DACDC_TIMER_PERIOD;
        ADCbufsizeMax = ADCdcBUF_SIZE_PIC;
        DACbufsizeMax = DACdcBUF_SIZE_PIC;
        break;
    default:
        break;
    }
    double actualSamplingInterval = MAX(ADCdt_min, MIN(targetSamplingInterval * SECONDS, MaxUpdateInterval * SECONDS));
    uint64_t minorDataEventInterval, majorDataEventInterval;

    /* Determine which slew mode to operate in */
    double ticksPerDACStep = abs(1 / dEdt / (calData->m_DACdcP_V / 2 + calData->m_DACdcN_V / 2) * SECONDS * 1000); //the 1000 factor accounts for the mV to V conversion, since dEdt is in mV/s
    pNode->DCsamplingParams.isFastSlewRate = ticksPerDACStep < ADCdt_min;

    if (pNode->DCsamplingParams.isFastSlewRate)
    {
        /* In fast slew mode, the DAC updates (N = ADC_DAC_ratio) times per minor data event.
            The ADC samples once per minor data event */

    /* Calculate DAC timing */
        pNode->DCsamplingParams.DACTimerDiv = 0;        //assume that at fast slew rates, only short timer periods will be needed (<32 bits)
        pNode->DCSweep_pot.VStep = 1;
        while ((pNode->DCsamplingParams.DACTimerPeriod = ticksPerDACStep * pNode->DCSweep_pot.VStep) < DACdt_min)
        {
            pNode->DCSweep_pot.VStep++;
        }

    /* Calculate ADC timing */
        /* minimize ADC_DAC_ratio to minimize minorDataEventInterval */
        pNode->DCsamplingParams.ADC_DAC_ratio = ceil((double)ADCdt_min / pNode->DCsamplingParams.DACTimerPeriod);
        minorDataEventInterval = pNode->DCsamplingParams.ADC_DAC_ratio * pNode->DCsamplingParams.DACTimerPeriod;

        /* ADCBufferSize * minorDataEventInterval = MajorDataEventInterval */
        pNode->DCsamplingParams.ADCBufferSizeEven = 1;
        while ((pNode->DCsamplingParams.ADCBufferSizeEven + 1) * minorDataEventInterval < actualSamplingInterval &&
            pNode->DCsamplingParams.ADCBufferSizeEven < ADCbufsizeMax)
        {
            pNode->DCsamplingParams.ADCBufferSizeEven++;
        }
        pNode->DCsamplingParams.minorPointsDiscarded = 0;
    }
    else
    {
        /* In slow slew mode, ADC samples at every minor data event, and averages (N = ADCbufSize) points
            The DAC updates every (N = DAC_ADC_ratio) times the ADCbuffer overflows */

    /* Calculate DAC timing */
        /* DAC is triggered at intervals of DAC_ADC_ratio * minorDataEventInterval * ADCbufSize * VStep */
        
        /* Adjust the sampling period if necessary so that ticksPerDACStep is an integer multiple */
        uint64_t compromisedSamplingInterval = ticksPerDACStep;
        if (actualSamplingInterval < ticksPerDACStep)
        {
            int N = 1;
            while ((compromisedSamplingInterval = ticksPerDACStep / N) > actualSamplingInterval)
                N++;
            pNode->DCsamplingParams.PointsSkippedMCU = N;
        }

        /* 1) Maximize ADCBufferSize, minimize minorDataEventInterval */
        pNode->DCsamplingParams.ADCBufferSizeEven = 1;
        pNode->DCsamplingParams.DAC_ADC_ratio = 1;
        pNode->DCSweep_pot.VStep = 1;
        while (floor((minorDataEventInterval = ticksPerDACStep / pNode->DCsamplingParams.ADCBufferSizeEven /
            pNode->DCsamplingParams.DAC_ADC_ratio) / ADCdt_min) > 1)
        {
                pNode->DCsamplingParams.ADCBufferSizeEven++;
        }
        while (pNode->DCsamplingParams.ADCBufferSizeEven > ADCbufsizeMax)
        {
            pNode->DCsamplingParams.DAC_ADC_ratio *= 2;
            pNode->DCsamplingParams.ADCBufferSizeEven /= 2;
        }
        minorDataEventInterval = ticksPerDACStep / pNode->DCsamplingParams.ADCBufferSizeEven / pNode->DCsamplingParams.DAC_ADC_ratio; //recalculate to minimize rounding error

    /* Calculate ADC timing */
        /* (1<<ADCTimerDiv) * ADCTimerPeriod = minorDataEventInterval */
        pNode->DCsamplingParams.ADCTimerDiv = 0;
        double ADCTimerPeriod = minorDataEventInterval;
        while (ADCTimerPeriod > 4294967295 - 5 * MILLISECONDS)
        {
            ADCTimerPeriod /= 2;
            pNode->DCsamplingParams.ADCTimerDiv++;
        }
        pNode->DCsamplingParams.ADCTimerPeriod = floor(ADCTimerPeriod);

        if (pNode->DCsamplingParams.DAC_ADC_ratio == 1)
            pNode->DCsamplingParams.minorPointsDiscarded = pNode->DCsamplingParams.ADCBufferSizeEven / 2;
        else
            pNode->DCsamplingParams.minorPointsDiscarded = 0;
    }

    /* PointsSkippedMCU * MajorDataEventInterval = actualSamplingInterval */
    majorDataEventInterval = pNode->DCsamplingParams.ADCBufferSizeEven * minorDataEventInterval;
    pNode->DCsamplingParams.PointsSkippedMCU = floor(actualSamplingInterval / majorDataEventInterval);

    /* actualSamplingInterval * PointsSkippedPC = targetSamplingInterval */
    pNode->DCsamplingParams.PointsSkippedPC = targetSamplingInterval / (majorDataEventInterval * pNode->DCsamplingParams.PointsSkippedMCU);

    pNode->DCsamplingParams.ADCcounter = 0;
    pNode->DCsamplingParams.ADCBufferSizeOdd = pNode->DCsamplingParams.ADCBufferSizeEven;
}

void ExperimentCalcHelperClass::GetSamplingParams_galvSweep(HardwareModel_t HWversion, const cal_t * calData, ExperimentNode_t * pNode,
    double dIdt, currentRange_t currentRange, double targetSamplingInterval, double MaxUpdateInterval)
{
    if (dIdt == 0)
    {
        GetSamplingParams_staticDAC(HWversion, pNode, targetSamplingInterval);
        return;
    }
    if (targetSamplingInterval == 0)
    {
        targetSamplingInterval = dIdt / 1000 * calData->m_DACdcP_I[(int)currentRange] * SECONDS;
    }

    int ADCdt_min, DACdt_min, ADCbufsizeMax, DACbufsizeMax;
    switch (HWversion)
    {
    case PRIME:
    case PICO:
    case EDGE:
    case SOLO:
    case PLUS:
        ADCdt_min = SQUIDSTAT_TEENSY_MIN_ADCDC_TIMER_PERIOD;
        DACdt_min = SQUIDSTAT_TEENSY_MIN_DACDC_TIMER_PERIOD;
        ADCbufsizeMax = ADCdcBUF_SIZE_TEENSY;
        DACbufsizeMax = DACdcBUF_SIZE_TEENSY;
        break;
    case PLUS_2_0:
    case PRIME_2_0:
    case SOLO_2_0:
        ADCdt_min = SQUIDSTAT_PIC_MIN_ADCDC_TIMER_PERIOD;
        DACdt_min = SQUIDSTAT_PIC_MIN_DACDC_TIMER_PERIOD;
        ADCbufsizeMax = ADCdcBUF_SIZE_PIC;
        DACbufsizeMax = DACdcBUF_SIZE_PIC;
        break;
    default:
        break;
    }
    double actualSamplingInterval = MAX(ADCdt_min, MIN(targetSamplingInterval * SECONDS, MaxUpdateInterval * SECONDS));
    uint64_t minorDataEventInterval, majorDataEventInterval;

    /* Determine which slew mode to operate in */
    double ticksPerDACStep = abs(1 / dIdt / (calData->m_DACdcP_I[(int)currentRange] / 2 + calData->m_DACdcN_I[currentRange] / 2) * SECONDS);
    pNode->DCsamplingParams.isFastSlewRate = ticksPerDACStep < ADCdt_min;

    if (pNode->DCsamplingParams.isFastSlewRate)
    {
        /* In fast slew mode, the DAC updates (N = ADC_DAC_ratio) times per minor data event.
        The ADC samples once per minor data event */

        /* Calculate DAC timing */
        pNode->DCsamplingParams.DACTimerDiv = 0;        //assume that at fast slew rates, only short timer periods will be needed (<32 bits)
        pNode->DCSweep_galv.IStep = 1;
        while ((pNode->DCsamplingParams.DACTimerPeriod = ticksPerDACStep * pNode->DCSweep_galv.IStep) < DACdt_min)
        {
            pNode->DCSweep_galv.IStep++;
        }

        /* Calculate ADC timing */
        /* minimize ADC_DAC_ratio to minimize minorDataEventInterval */
        pNode->DCsamplingParams.ADC_DAC_ratio = ceil((double)ADCdt_min / pNode->DCsamplingParams.DACTimerPeriod);
        minorDataEventInterval = pNode->DCsamplingParams.ADC_DAC_ratio * pNode->DCsamplingParams.DACTimerPeriod;

        /* ADCBufferSize * minorDataEventInterval = MajorDataEventInterval */
        pNode->DCsamplingParams.ADCBufferSizeEven = 1;
        while ((pNode->DCsamplingParams.ADCBufferSizeEven + 1) * minorDataEventInterval < actualSamplingInterval &&
            pNode->DCsamplingParams.ADCBufferSizeEven < ADCbufsizeMax)
        {
            pNode->DCsamplingParams.ADCBufferSizeEven++;
        }
        pNode->DCsamplingParams.minorPointsDiscarded = 0;
    }
    else
    {
        /* In slow slew mode, ADC samples at every minor data event, and averages (N = ADCbufSize) points
        The DAC updates every (N = DAC_ADC_ratio) times the ADCbuffer overflows */

        /* Calculate DAC timing */
        /* DAC is triggered at intervals of DAC_ADC_ratio * minorDataEventInterval * ADCbufSize * IStep */

        /* Adjust the sampling period if necessary so that ticksPerDACStep is an integer multiple */
        uint64_t compromisedSamplingInterval = ticksPerDACStep;
        if (actualSamplingInterval < ticksPerDACStep)
        {
            int N = 1;
            while ((compromisedSamplingInterval = ticksPerDACStep / N) > actualSamplingInterval)
                N++;
            pNode->DCsamplingParams.PointsSkippedMCU = N;
        }

        /* 1) Maximize ADCBufferSize, minimize minorDataEventInterval */
        pNode->DCsamplingParams.ADCBufferSizeEven = 1;
        pNode->DCsamplingParams.DAC_ADC_ratio = 1;
        pNode->DCSweep_galv.IStep = 1;
        while (floor((minorDataEventInterval = ticksPerDACStep / pNode->DCsamplingParams.ADCBufferSizeEven /
            pNode->DCsamplingParams.DAC_ADC_ratio) / ADCdt_min) > 1)
        {
            pNode->DCsamplingParams.ADCBufferSizeEven++;
        }
        while (pNode->DCsamplingParams.ADCBufferSizeEven > ADCbufsizeMax)
        {
            pNode->DCsamplingParams.DAC_ADC_ratio *= 2;
            pNode->DCsamplingParams.ADCBufferSizeEven /= 2;
        }
        minorDataEventInterval = ticksPerDACStep / pNode->DCsamplingParams.ADCBufferSizeEven / pNode->DCsamplingParams.DAC_ADC_ratio; //recalculate to minimize rounding error

                                                                                                                                      /* Calculate ADC timing */
                                                                                                                                      /* (1<<ADCTimerDiv) * ADCTimerPeriod = minorDataEventInterval */
        pNode->DCsamplingParams.ADCTimerDiv = 0;
        double ADCTimerPeriod = minorDataEventInterval;
        while (ADCTimerPeriod > 4294967295 - 5 * MILLISECONDS)
        {
            ADCTimerPeriod /= 2;
            pNode->DCsamplingParams.ADCTimerDiv++;
        }
        pNode->DCsamplingParams.ADCTimerPeriod = floor(ADCTimerPeriod);

        if (pNode->DCsamplingParams.DAC_ADC_ratio == 1)
            pNode->DCsamplingParams.minorPointsDiscarded = pNode->DCsamplingParams.ADCBufferSizeEven / 2;
        else
            pNode->DCsamplingParams.minorPointsDiscarded = 0;
    }

    /* PointsSkippedMCU * MajorDataEventInterval = actualSamplingInterval */
    majorDataEventInterval = pNode->DCsamplingParams.ADCBufferSizeEven * minorDataEventInterval;
    pNode->DCsamplingParams.PointsSkippedMCU = floor(actualSamplingInterval / majorDataEventInterval);

    /* actualSamplingInterval * PointsSkippedPC = targetSamplingInterval */
    pNode->DCsamplingParams.PointsSkippedPC = targetSamplingInterval / (majorDataEventInterval * pNode->DCsamplingParams.PointsSkippedMCU);

    pNode->DCsamplingParams.ADCcounter = 0;
    pNode->DCsamplingParams.ADCBufferSizeOdd = pNode->DCsamplingParams.ADCBufferSizeEven;
}

void ExperimentCalcHelperClass::GetSamplingParameters_pulse(HardwareModel_t HWversion, double t_period, double t_pulsewidth, ExperimentNode_t * pNode)
{
  pNode->DCsamplingParams.PointsSkippedPC = 1;
  int dt_min = 50000;
  int ADCbufsize = 256;
  int DACbufsize = 256;
  switch (HWversion)
  {
  case PRIME:
  case PICO:
  case EDGE:
  case SOLO:
  case PLUS:
    dt_min = SQUIDSTAT_TEENSY_MIN_ADCDC_TIMER_PERIOD;
    ADCbufsize = ADCdcBUF_SIZE_TEENSY;
    DACbufsize = DACdcBUF_SIZE_TEENSY;
    break;
  case PLUS_2_0:
  case PRIME_2_0:
  case SOLO_2_0:
    dt_min = SQUIDSTAT_PIC_MIN_ADCDC_TIMER_PERIOD;
    ADCbufsize = ADCdcBUF_SIZE_PIC;
    DACbufsize = DACdcBUF_SIZE_PIC;
    break;
  default:
    break;
  }

  pNode->DCsamplingParams.ADCTimerDiv = 0;
  pNode->DCsamplingParams.DAC_ADC_ratio = 1;

  /* 1) Take the lesser of (period - pulsewidth) and pulsewidth */
  double t_pulse;
  bool isEvenPeriodShorter;
  if (t_period - t_pulsewidth < t_pulsewidth)
  {
    t_pulse = t_pulsewidth;
    isEvenPeriodShorter = true;
  }
  else
  {
    t_pulse = t_period - t_pulsewidth;
    isEvenPeriodShorter = false;
  }
  uint16_t bufMult = 1;

  /* 2) Minimize dt */
  uint32_t dt = t_pulse * MILLISECONDS / bufMult;
  while (dt / dt_min > 1 && bufMult < DACbufsize && bufMult < ADCbufsize)
  {
    bufMult <<= 1;  //should this be bufMult++ ?
    dt = t_pulse * MILLISECONDS / bufMult;
  }
  pNode->DCsamplingParams.ADCTimerPeriod = dt;

  if (isEvenPeriodShorter)
  {
    pNode->DCsamplingParams.ADCBufferSizeOdd = bufMult;
    pNode->DCsamplingParams.ADCBufferSizeEven = pNode->DCsamplingParams.ADCBufferSizeOdd * (t_period - t_pulsewidth) / t_pulsewidth;
  }
  else
  {
    pNode->DCsamplingParams.ADCBufferSizeEven = bufMult;
    pNode->DCsamplingParams.ADCBufferSizeOdd = pNode->DCsamplingParams.ADCBufferSizeEven * (t_pulsewidth / (t_period - t_pulsewidth));
  }
  pNode->DCsamplingParams.minorPointsDiscarded = isEvenPeriodShorter ? pNode->DCsamplingParams.ADCBufferSizeEven / 2 : pNode->DCsamplingParams.ADCBufferSizeOdd / 2;
}

currentRange_t ExperimentCalcHelperClass::GetMinCurrentRange(HardwareModel_t HWversion, const cal_t * calData, double targetCurrent)
{
	int range;
	switch (HWversion)
	{
	case PRIME:
	case EDGE:
	case PICO:
	case SOLO:
  case PLUS:
    range = 3;
		break;
	case PLUS_2_0:
	case SOLO_2_0:
	case PRIME_2_0:
    range = 7;
		break;
	default:
		break;
	}

  while (1)
  {
    float slope = MAX(calData->m_iN[range], calData->m_iP[range]);
    if (fabs(targetCurrent) > fabs(slope * OVERCURRENT_LIMIT + calData->b_i[range]) && range > 0)
      range--;
    else
      break;
  }
  return (currentRange_t)range;
}

int16_t ExperimentCalcHelperClass::GetBINCurrent(const cal_t * calData, currentRange_t currentRange, double targetCurrent)
{
	float slope = targetCurrent > 0 ? calData->m_DACdcP_I[(int)currentRange] : calData->m_DACdcN_I[(int)currentRange];
	int32_t binCurrent = (int32_t)(targetCurrent * slope + calData->b_DACdc_I[(int)currentRange]);
	binCurrent = MIN(MAX(binCurrent, -32768), 32767);
	return binCurrent;
}

int16_t ExperimentCalcHelperClass::GetBINVoltageForDAC(const cal_t * calData, double targetVoltage)
{
	float slope = targetVoltage > 0 ? calData->m_DACdcP_V : calData->m_DACdcN_V;
	int32_t binVolt = (int32_t)(targetVoltage * slope + calData->b_DACdc_V);
	binVolt = MIN(MAX(binVolt, -32768), 32767);
	return (int16_t)binVolt;
}

ProcessedDCData ExperimentCalcHelperClass::ProcessDCDataPoint(const cal_t * calData, ExperimentalDcData rawData)
{
  ProcessedDCData processedData;
  double ewe = rawData.ADCrawData.ewe > 0 ? calData->m_eweP * rawData.ADCrawData.ewe + calData->b_ewe : calData->m_eweN * rawData.ADCrawData.ewe + calData->b_ewe;
  double ref = rawData.ADCrawData.ref > 0 ? calData->m_refP * rawData.ADCrawData.ref + calData->b_ref : calData->m_refN * rawData.ADCrawData.ref + calData->b_ref;
  double ece = rawData.ADCrawData.ece > 0 ? calData->m_eceP * rawData.ADCrawData.ece + calData->b_ece : calData->m_eceN * rawData.ADCrawData.ece + calData->b_ece;
  processedData.EWE = ewe - ref;
  processedData.ECE = ece - ref;
  int n = (int)rawData.currentRange;
  if (rawData.currentRange == OFF)

    processedData.current = 0;
  else
    processedData.current = rawData.ADCrawData.current > 0 ? calData->m_iP[(int)rawData.currentRange] * rawData.ADCrawData.current + calData->b_i[(int)rawData.currentRange] :
                                                           calData->m_iN[(int)rawData.currentRange] * rawData.ADCrawData.current + calData->b_i[(int)rawData.currentRange];
  return processedData;
}

double ExperimentCalcHelperClass::GetUnitsMultiplier(QString units_str)
{
  /* current units */
  if (units_str.contains("mA"))
    return 1;
  else if (units_str.contains("uA"))
    return 1e-3;
  else if (units_str.contains("nA"))
    return 1e-6;

  /* frequency units */
  else if (units_str.contains("mHz"))
    return 1e-3;
  else if (units_str.contains("kHz"))
    return 1e3;
  else if (units_str.contains("MHz"))
    return 1e6;
  else if (units_str.contains("Hz"))    //this else-if must come after the other xHz units
    return 1;

  /* resistance units */
  else if (units_str.contains("kOhms"))
    return 1e3;
  else if (units_str.contains("MOhms"))
    return 1e6;
  else if (units_str.contains("Ohms"))
    return 1;

  /* power units */
  else if (units_str.contains("nW"))
    return 1e-6;
  else if (units_str.contains("uW"))
    return 1e-3;
  else if (units_str.contains("mW"))
    return 1;
  else if (units_str.contains("W"))
    return 1e3;

  /* time units */
  else if (units_str.contains("min"))
    return 60;
  else if (units_str.contains("hr"))
    return 3600;
  else
    return 1;
}

/* AC methods */

currentRange_t ExperimentCalcHelperClass::GetMinCurrentRange_DACac(const cal_t * calData, double targetCurrentAmp)
{
  int range = 7;

  while (1)
  { 
    if (ABS(targetCurrentAmp * calData->m_DACdcP_I[range] / calData->m_DACdcP_V) > 3.3 / 2)
      range--;
    else
      break;
  }
  return (currentRange_t)range;
}

QList<double> ExperimentCalcHelperClass::calculateFrequencyList(double lowerFreq, double upperFreq, double pointsPerDecade)
{
  double _pointsPerDecade = ABS(pointsPerDecade);
  double _upperFreq = MAX(upperFreq, lowerFreq);
  double _lowerFreq = MIN(upperFreq, lowerFreq);
  double NumSamples = pointsPerDecade * log10(_upperFreq / _lowerFreq) + 1;
  int NumSamplesInteger = (int)ceil(NumSamples);
  QList<double> frequencies;

  double ratio = pow(10, -1.0 / _pointsPerDecade);

  for (int i = 0; i < NumSamplesInteger - 1; i++)
  {
    double x = _upperFreq * pow(ratio, i);
    frequencies.append(x);
  }
  frequencies.append(_lowerFreq);
  return frequencies;
}

void ExperimentCalcHelperClass::calcACSamplingParams(const cal_t * calData, ExperimentNode_t * pNode)
{
  pNode->ACsamplingParams.freqRange = pNode->ACsamplingParams.frequency > HF_CUTOFF_VALUE ? HF_RANGE : LF_RANGE;

  /* (1) Calculate signal gen clock frequency and signal gen register value */
  int wavegenClkdiv = 1 << (int) WAVEGENCLK_24KHZ;
  pNode->ACsamplingParams.wavegenClkSpeed = WAVEGENCLK_24KHZ;
  double fSignal = pNode->ACsamplingParams.frequency;

  while (fSignal * SIGNAL_GEN_RESOLUTION * wavegenClkdiv >= 25e6)
  {
    wavegenClkdiv >>= 1;
    pNode->ACsamplingParams.wavegenClkSpeed = (WAVEGENCLK_SPEED_t)((int)pNode->ACsamplingParams.wavegenClkSpeed - 1);
    if (pNode->ACsamplingParams.wavegenClkSpeed == WAVEGENCLK_25MHZ)
      break;
  }
  pNode->ACsamplingParams.wavegenFraction = (uint32_t)(fSignal * wavegenClkdiv / 25e6 * 268435456);
  pNode->ACsamplingParams.wavegenFraction = (pNode->ACsamplingParams.wavegenFraction == 0) ? 1 : pNode->ACsamplingParams.wavegenFraction; //make sure frequency doesn't equal zero

  /* Re-calculate signal frequency based on integer timer values */
  fSignal = (double)pNode->ACsamplingParams.wavegenFraction / 268435456.0 * 25e6 / wavegenClkdiv;
  pNode->ACsamplingParams.frequency = fSignal;

  /* (2) Calculate ADCac buffer size and sampling frequency*/
  int n = ADCacBUF_SIZE;
  double fSample;
  uint32_t ADCclkdiv = 1;

  if (pNode->ACsamplingParams.freqRange == HF_RANGE)
    fSample = (n - 1) * fSignal / n;
  else
    fSample = fSignal * n / 2;
  uint64_t TimerPeriod = (uint64_t)(100.0e6 / fSample / ADCclkdiv);

  while (1)
  {
    while (TimerPeriod > 4294967296)
    {
      pNode->ACsamplingParams.ADCacTimerClkDiv++;
      TimerPeriod >>= 1;
      ADCclkdiv <<= 1;
    }

    /* Recalculate sampling frequency and ADCbufsize based on integer timer values */
    fSample = 100.0e6 / ADCclkdiv / TimerPeriod;
    if (pNode->ACsamplingParams.freqRange == HF_RANGE)
    {
      double denom = fSignal - fSample;
      double x = ABS(fSignal / denom);
      if (fSample > fSignal || denom == 0 || x > ADCacBUF_SIZE / 1.5)   //debugging: try to get at least 1.5 cycles, for fitting
      {
        TimerPeriod++;
        continue;
      }
      else
        break;
    }
    else
      break;
  }
  pNode->ACsamplingParams.ADCacTimerPeriod = (uint32_t)TimerPeriod;

  //debugging
  //todo: make algorithm for calculating num buffers
  pNode->ACsamplingParams.numBufs = 20;
}

double ExperimentCalcHelperClass::estimatePeriod(const ExperimentalAcData acDataHeader)
{
  double samplingFreq = 1.0e8 / (1 << acDataHeader.ADCacTimerDiv) / acDataHeader.ADCacTimerPeriod;
  freq_range_t freqRange = acDataHeader.frequency > HF_CUTOFF_VALUE ? HF_RANGE : LF_RANGE;
  double samplesPerPeriod;

  if (freqRange == HF_RANGE)
  {
     samplesPerPeriod = acDataHeader.frequency / (acDataHeader.frequency - samplingFreq);
  }
  else
  {
    samplesPerPeriod = samplingFreq / acDataHeader.frequency;
  }

  return samplesPerPeriod;
}

/* Sinusoidal curve fitting */
ComplexDataPoint_t ExperimentCalcHelperClass::AnalyzeFRA(double frequency, uint16_t * rawDataBuf, uint8_t numACBuffers, double gainEWE, double gainI, double approxPeriod, const cal_t * calData, currentRange_t range)
{
    int len = numACBuffers * ADCacBUF_SIZE;
    QVector<double> rawIData, rawVData, filteredIData, filteredVData;

    for (int i = 0; i < numACBuffers; i++)
    {
        for (int j = 0; j < ADCacBUF_SIZE; j++)
        {
            rawIData.append(rawDataBuf[2 * i * ADCacBUF_SIZE + j]);
            rawVData.append(rawDataBuf[(2 * i + 1)*ADCacBUF_SIZE + j]);
        }
    }

    int rollingFilterSize = MAX(1, approxPeriod / 5);
    double Period_result, Period_resultPrev = approxPeriod, fractionalChange;

    do {
        filteredIData = rollingAverage(rawIData, rollingFilterSize);
        filteredVData = rollingAverage(rawVData, rollingFilterSize);
        Period_result = (GetPeriod(filteredIData) + GetPeriod(filteredVData)) / 2;
        fractionalChange = abs(Period_result - Period_resultPrev) / Period_result;
        Period_resultPrev = Period_result;
        rollingFilterSize = MAX(1, Period_result / 5);
    } while (fractionalChange > 0.0001);

    /******************************************************/
    /* debugging only */
    std::ofstream fout;
    QString filename = "C:/Users/Matt/Desktop/results";
    filename.append(QString::number(frequency));
    filename.append(".txt");
    fout.open(filename.toStdString(), std::ofstream::out);
    fout << "Period width = " << Period_result << '\n';
    for (int i = 0; i < filteredIData.count(); i++)
    {
        fout << filteredIData[i] << '\t' << filteredVData[i] << '\n';
    }
    /******************************************************/

    int truncatedLen = (int)(floor(len / Period_result) * Period_result);
    ComplexDataPoint_t Ipt = SingleFrequencyFourier(rawIData, truncatedLen, Period_result);
    ComplexDataPoint_t Vpt = SingleFrequencyFourier(rawVData, truncatedLen, Period_result);
    Ipt.ImpedanceMag /= gainI * fabs(calData->m_DACdcP_I[range]) * 1000;
    Vpt.ImpedanceMag /= gainEWE * fabs(calData->m_DACdcP_V);
    ComplexDataPoint_t Z;
    Z.ImpedanceMag = Vpt.ImpedanceMag / Ipt.ImpedanceMag;
    Z.phase = Ipt.phase - Vpt.phase;
    Z.ImpedanceReal = Z.ImpedanceMag * cos(Z.phase * M_PI / 180);
    Z.ImpedanceImag = Z.ImpedanceMag * sin(Z.phase * M_PI / 180);
    Z.frequency = frequency;

    return Z;
}

//=============================================================================
namespace // unnamed namespace to make these function definitions private within this file
{
    // func Average: Computes the average of a vector.
    //
    // in     x: 
    // returns: The average of x.
    double Average(QVector<double> const& x)
    {
        if (x.size() < 1) { return 0; }

        double sum = 0;
        for (int i = 0; i < x.size(); i++)
        {
            sum += x[i];
        }
        double average = sum / x.size();
        return average;
    }

    // func Zero: Computes a "zero-value" of a vector.
    //
    // in     x: 
    // returns: The zero-value of x.
    double Zero(QVector<double> const& x)
    {
        // CHOOSE: find the zero by taking the average
        double zero = Average(x);

        std::cout << "Zero: " << zero << std::endl;
        return zero;
    }

    // func Sign: Computes the sign (pos, neg, zero) of a number.
    //
    // in     x: 
    // returns: -1 <- neg, 0 <- 0, +1 <- pos
    int Sign(int x)
    {
        int sign = (x > 0) - (x < 0);
        return sign;
    }

    // func AllZeroCrossings: Finds indicies of all zero-crossing leading edges.
    //
    //    out zc: indicies of lead-edges of zero-crossings.
    // in     x:
    // in     zero: 
    void AllZeroCrossings(QVector<int>& zca, QVector<double> const& x, double zero)
    {
        if (!zca.empty() || x.size() < 2)
        {
            std::cout << "AllZeroCrossings" << std::endl;
            exit(EXIT_FAILURE);
        }

        // process all but the last point
        for (int i = 0; i < x.size() - 1; i++)
        {
            // vertical shift each point relative to the zero-value
            int sign_curr = Sign(x[i] - zero);
            int sign_next = Sign(x[i + 1] - zero);
            if ((sign_curr == 0) || (sign_curr != sign_next)) {
                zca.push_back(i);
            }
        }

        // process the last point
        if (x.back() - zero == 0)
        {
            // only add if the last point is equal to the zero-value
            zca.push_back(x.back());
        }

        std::cout << "zca: ";
        for (int i = 0; i < zca.size(); i++) {
            std::cout << zca[i] << ",";
        }
        std::cout << std::endl;
    }

    // func Edge1: Determines the "cutoff" point between zero-cross intervals
    // that are small or large.2 "buckets" (small/large), 1 edge between them.
    // in     zc: Indices of zero-crossings.
    // returns: The edge (cutoff) value separating a small zc-interval from a large one.
    int Edge1(QVector<int> const& zc)
    {
        // find the maximum distance between zero-crossings
        // by computing the distance between each adjacent zc
        // and keep track of the maximum
        int max = 0;
        for (int i = 0; i < zc.size() - 1; i++)
        {
            int dist = zc[i + 1] - zc[i];
            if (dist > max)
            {
                max = dist;
            }
        }

        // CHOOSE: edge is half of max
        int edge = max / 2;
        std::cout << "max run: " << max << std::endl;
        std::cout << "edge: " << edge << std::endl;

        return edge;
    }

    // func GroupZeroCrossings: Gets start and stop indices of zero-cross clusters.
    //
    //    out zci: (start,stop) indices of zero-crossing interval.
    // in     zca: Indices of all zero-crossing leading-edges.
    // in     nx: The original sample size from which zca was computed.
    void ZeroCrossIntervals(QVector< QPair<int, int> >& zci, QVector<int> const& zca, int nx)
    {
        // nx     = 21 <-- original sample size
        // zca    = 3  4  10  15 16 20 <-- leading edges of all zero-crossings in the original sample of size nx
        // edge   = 2 <-- anything <= is a cluster, anything > is not a cluster
        // zci[0] = [3, 5] <-- trailing edge (5) is one more than the last leading edge (4) in the cluster
        // zci[1] = [10, 11] <-- not a cluster
        // zci[2] = [15, 17] <-- is a cluster
        // zci[3] = [20, 20] <-- trailing edge = leading edge because the original sample is of size nx=21
        
        if (!zci.empty() || zca.size() < 2) {
            std::cout << "ZeroCrossIntervals" << std::endl;
            exit(EXIT_FAILURE);
        }

        // CHOOSE: two "buckets"
        int edge = Edge1(zca);

        // distances between zero-crossings
        int i = 0;
        int beg = zca[i];
        int end = beg;
        int n = zca.size();
        // process all but the last point since we're using a forward difference
        while (i < (n - 1)) {
            // save the starting index when you encounter any type of zero-crossing
            beg = zca[i]; // leading-edge of zero-crossing

            int dist = 0;
            bool at_end;
            bool in_cluster = false;
            // keep updating the trailing-edge index until you exit the cluster.
            // this will only happen once if the zero-crossing is a lone point (not a cluster)
            do
            {
                dist = zca[i + 1] - zca[i]; // forward difference
                i += 1;
                at_end = i >= (n - 1);
                in_cluster = dist < edge; // CHOOSE: non-inclusive
            } while (!at_end && in_cluster);
            
            // we're out of the cluster or past the lone zero-crossing point,
            // so save the trailing-edge index, which is one more than the last leading-edge index
            end = zca[i - 1] + 1; // trailing-edge of zero-crossing

            // store the interval
            QPair<int, int> interval(beg, end);
            zci.push_back(interval);
        }

        // process the last point, which must be done after the loop
        // since we're using a forward-difference
        beg = zca[i];
        if (i == nx - 1) {
            // the last zero-crossing is the last point of the original vector,
            // so the trailing edge is also the leading edge,
             // which happens when the last point is equal to the zero-value
            end = beg;
        }
        else
        {
            // the last zero-crossing is not the last point of the original vector,
            // so it's ok to use the following point as the zero-crossing trailing-edge
            end = beg + 1;
        }
    
        QPair<int, int> interval(beg, end);
        zci.push_back(interval);

        std::cout << "zci: ";
        for (int i = 0; i < zci.size(); i++) {
            std::cout << zci[i].first << ":" << zci[i].second << ",";
        }
        std::cout << std::endl;
    }

    int AverageDistance(QVector< std::pair<int, int> >& x)
    {
        int average_distance = 0;
        for (int i = 0; i < x.size(); i++)
        {
            int dist = x[i].second - x[i].first;
            average_distance += dist / (i + 1);
        }
        return average_distance;
    }

    //double AverageDistance(QVector<int> const& x)
    //{
    //    // backward-difference
    //    double average_distance = 0;
    //    for (int i = 1; i < x.size(); i++)
    //    {
    //        average_distance += ((double)(x[i] - x[i - 1])) / i;
    //    }
    //    return average_distance;
    //}
    double AverageDistance(QVector<int> const& x)
    {
        /* Measure using an odd number of zero-crossings, in case the zero-point does not equal the sine's centerline */
        int numCrossings = x.count() % 2 == 1 ? x.count() : x.count() - 1;
        return ((double)(x[numCrossings - 1] - x[0])) / (numCrossings - 1);
    }

    void SmoothZeroCrossings(QVector<int>& zca, QVector< QPair<int, int> > const& zci)
    {
        for (int i = 0; i < zci.size(); i++)
        {
            // CHOOSE: floor since lead-edge defines the zero-crossing
            int zc = (zci[i].second + zci[i].first) / 2;
            zca.push_back(zc);
        }
    }
} // namespace

// in     xbuf: Must already be smoothed
double ExperimentCalcHelperClass::GetPeriod(QVector<double> const x)
{
    // Algorithm Variables
    //std::vector<int> x(xbuf_smoothed, xbuf_smoothed + size * sizeof(xbuf_smoothed[0])); // copy xbuf array into x vector
    
    QVector<int> zca; // indicies of all zero-crossings
    QVector< QPair<int, int> > zci; // index-pairs of zero-crossing intervals
    double zero = 0; // computed "zero-value"

    // choose a zero-value
    zero = Zero(x);

    // find all zero-crossings
    AllZeroCrossings(zca, x, zero);

    // find zero-crossing intervals
    ZeroCrossIntervals(zci, zca, x.size());

    // overwrite zca with clusters resolved to single points
    zca.clear();
    SmoothZeroCrossings(zca, zci);

    double halfperiod = AverageDistance(zca);
    return 2 * halfperiod;
}

ComplexDataPoint_t ExperimentCalcHelperClass::SingleFrequencyFourier(QVector<double> data, int len, double period)
{
    double sumSine = 0, sumCosine = 0;
    for (int i = 0; i < len; i++)
    {
        double arg = i * (2 * M_PI / period);
        sumSine += data[i] * sin(arg);
        sumCosine += data[i] * cos(arg);
    }
    ComplexDataPoint_t pt;
    pt.ImpedanceImag = sumSine;
    pt.ImpedanceReal = sumCosine;
    pt.ImpedanceMag = sqrt(sumSine * sumSine + sumCosine * sumCosine);
    pt.phase = atan2(sumSine, sumCosine) * 180 / M_PI;
    return pt;
}

QVector<double> ExperimentCalcHelperClass::rollingAverage(QVector<double> rawData, int rollingAvgWidth)
{
    if (rollingAvgWidth <= 1)
        return rawData;

    /* Pad the raw data with zeroes on the front and back*/
    for (int i = 0; i < rollingAvgWidth; i++)
    {
        rawData.push_front(0);
        rawData.push_back(0);
    }

    /* Compute the rolling average */
    double rollingSum = 0;
    QVector<double> smoothedData;
    for (int n = 0; n < rawData.count() - rollingAvgWidth; n++)
    {
        rollingSum += rawData[n + rollingAvgWidth] - rawData[n];
        smoothedData.append(rollingSum / rollingAvgWidth);
    }

    /* Remove padding, throw away "garbage points" */
    for (int i = 0; i < rollingAvgWidth; i++)
    {
        rawData.pop_front();
        smoothedData.pop_front();
        rawData.pop_back();
        smoothedData.pop_back();
    }

    return smoothedData;
}
