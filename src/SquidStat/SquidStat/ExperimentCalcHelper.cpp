#define _USE_MATH_DEFINES

#include "ExperimentCalcHelper.h"
#include "Log.h"  //debugging only
#include <qdebug.h>  //debugging only
#include <iostream> //debugging only
#include <fstream> //debugging only

/* DC methods */

void ExperimentCalcHelperClass::GetSamplingParams_staticDAC(HardwareModel_t HWversion, ExperimentNode_t * pNode, const double targetSamplingInterval, double MaxUpdateInterval)
{
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

//TODO: incorporate fastslew mode, PointsSkippedMCU
void ExperimentCalcHelperClass::GetSamplingParams_potSweep(HardwareModel_t HWversion, const cal_t * calData, ExperimentNode_t * pNode,
  double dEdt, const double targetSamplingInterval)
{
  if (dEdt == 0) return;

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

  pNode->DCsamplingParams.isFastSlewRate = false;  //debugging
  pNode->DCsamplingParams.PointsSkippedMCU = 1;  //debugging: todo, incorporate these params into fast slewrate notes

  /* 1) Minimize dt, maximize DACMult */
  pNode->DCsamplingParams.DACMultEven = pNode->DCsamplingParams.DACMultOdd = 1;
  pNode->DCSweep_pot.VStep = 1;
  uint64_t dt;
  double ticksPerStep = abs(1 / dEdt / (calData->m_DACdcP_V / 2 + calData->m_DACdcN_V / 2) * SECONDS * 1000); //the 1000 factor accounts for the mV to V conversion, since dEdt is in mV/s
  do
  {
    dt = (uint64_t)round(ticksPerStep / pNode->DCsamplingParams.DACMultEven);
    if (dt / dt_min > 1 && pNode->DCsamplingParams.DACMultEven < DACbufsize)
    {
      if (pNode->DCsamplingParams.DACMultEven << 1 < DACbufsize)
      {
        pNode->DCsamplingParams.DACMultEven <<= 1;
        pNode->DCsamplingParams.DACMultOdd <<= 1;
      }
      else
        break;
    }
  } while (dt / dt_min > 1 && pNode->DCsamplingParams.DACMultEven < DACbufsize);

  /* 2) Increase VStep, if necessary (if dt is too small, or if DACbuf is expended too quickly */
  while (dt < dt_min || pNode->DCsamplingParams.DACMultEven * dt * DACbufsize < MIN_TICKS_FOR_USB_TRANSMISSION)
  {
    pNode->DCSweep_pot.VStep++;
    dt = (uint64_t)round(ticksPerStep / pNode->DCsamplingParams.DACMultEven * pNode->DCSweep_pot.VStep);
  }

  /* 3) Calculate ADCBufferSizeEven and Odd (must be <= ADCbufsize). Data should be sent no less than once per second */
  /* if auto-calculate mode is selected (targetSamplingInterval == 0), then use samplingInterval = dt * DACdcBUF_SIZE */
  double transmitInterval;
  uint32_t FilterSize = 0;
  do
  {
    FilterSize++;
    pNode->DCsamplingParams.ADCBufferSizeEven = targetSamplingInterval == 0 ? (pNode->DCsamplingParams.DACMultEven / FilterSize) :
      targetSamplingInterval * SECONDS / dt / FilterSize;
    transmitInterval = pNode->DCsamplingParams.ADCBufferSizeEven * dt;
  } while ((pNode->DCsamplingParams.ADCBufferSizeEven > ADCbufsize) || (transmitInterval > 1 * SECONDS));
  pNode->DCsamplingParams.ADCBufferSizeOdd = pNode->DCsamplingParams.ADCBufferSizeEven;
  pNode->DCsamplingParams.PointsSkippedPC = FilterSize;

  /* 4) Calculate Points Ignored (for dynamic sampling) */
  if (pNode->DCsamplingParams.ADCBufferSizeEven == pNode->DCsamplingParams.DACMultEven)
    pNode->DCsamplingParams.minorPointsDiscarded = pNode->DCsamplingParams.ADCBufferSizeEven / 2;
  else
    pNode->DCsamplingParams.minorPointsDiscarded = 0;

  /* 5) Make sure timer period isn't too big for uint32_t */
  pNode->DCsamplingParams.ADCTimerDiv = 0;
  while (dt > 4294967295 - 5 * MILLISECONDS) //5ms accounts conservatively for loop time
  {
    pNode->DCsamplingParams.ADCTimerDiv++;
    dt >>= 1;
  }
  pNode->DCsamplingParams.ADCTimerPeriod = (uint32_t) dt;
}

//TODO: incorporate fast slew mode, PointsSkippedMCU
void ExperimentCalcHelperClass::GetSamplingParams_galvSweep(HardwareModel_t HWversion, const cal_t * calData, ExperimentNode_t * pNode,
  double dIdt, currentRange_t currentRange, const double targetSamplingInterval)
{
  if (dIdt == 0) return;

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

  pNode->DCsamplingParams.isFastSlewRate = false; //debugging. todo: incorporate fast sampling mode

  /* 1) Minimize dt, maximize DACMult */
  pNode->DCsamplingParams.DACMultEven = pNode->DCsamplingParams.DACMultOdd = 1;
  pNode->DCSweep_galv.IStep = 1;
  uint64_t dt;
  double ticksPerStep = abs(1 / dIdt / (calData->m_DACdcP_I[(int)currentRange] / 2 + calData->m_DACdcN_I[currentRange] / 2) * SECONDS);
  do
  {
    dt = (uint64_t)round(ticksPerStep / pNode->DCsamplingParams.DACMultEven);
    if (dt / dt_min > 1 && pNode->DCsamplingParams.DACMultEven < DACbufsize)
    {
      if (pNode->DCsamplingParams.DACMultEven << 1 < DACbufsize)
      {
        pNode->DCsamplingParams.DACMultEven <<= 1;
        pNode->DCsamplingParams.DACMultOdd <<= 1;
      }
      else
        break;
    }
  } while (dt / dt_min > 1 && pNode->DCsamplingParams.DACMultEven < DACbufsize);

  /* 2) Increase VStep, if necessary (if dt is too small, or if DACbuf is expended too quickly */
  while (dt < dt_min || pNode->DCsamplingParams.DACMultEven * dt * DACbufsize < MIN_TICKS_FOR_USB_TRANSMISSION)
  {
    pNode->DCSweep_galv.IStep++;
    dt = (uint64_t)round(ticksPerStep / pNode->DCsamplingParams.DACMultEven * pNode->DCSweep_galv.IStep);
  }

  /* 3) Calculate ADCBufferSizeEven and Odd (must be <= ADCbufsize). Data should be sent no less than once per second */
  /* if auto-calculate mode is selected (targetSamplingInterval == 0), then use samplingInterval = dt * DACdcBUF_SIZE */
  double transmitInterval;
  uint32_t FilterSize = 0;
  do
  {
    FilterSize++;
    pNode->DCsamplingParams.ADCBufferSizeEven = targetSamplingInterval == 0 ? (pNode->DCsamplingParams.DACMultEven / FilterSize) :
      targetSamplingInterval * SECONDS / dt / FilterSize;
    transmitInterval = pNode->DCsamplingParams.ADCBufferSizeEven * dt;
  } while ((pNode->DCsamplingParams.ADCBufferSizeEven > ADCbufsize) || (transmitInterval > 1 * SECONDS));
  pNode->DCsamplingParams.ADCBufferSizeOdd = pNode->DCsamplingParams.ADCBufferSizeEven;
  pNode->DCsamplingParams.PointsSkippedPC = FilterSize;

  /* 4) Calculate Points Ignored (for dynamic sampling) */
  if (pNode->DCsamplingParams.ADCBufferSizeEven == pNode->DCsamplingParams.DACMultEven)
    pNode->DCsamplingParams.minorPointsDiscarded = pNode->DCsamplingParams.ADCBufferSizeEven / 2;
  else
    pNode->DCsamplingParams.minorPointsDiscarded = 0;

  /* 5) Make sure timer period isn't too big for uint32_t */
  pNode->DCsamplingParams.ADCTimerDiv = 0;
  while (dt > 4294967295 - 5 * MILLISECONDS) //5ms accounts conservatively for loop time
  {
    pNode->DCsamplingParams.ADCTimerDiv++;
    dt >>= 1;
  }
  pNode->DCsamplingParams.ADCTimerPeriod = (uint32_t)dt;
  pNode->DCsamplingParams.PointsSkippedPC = FilterSize;
}

void ExperimentCalcHelperClass::GetSamplingParameters_pulse(HardwareModel_t HWversion, double t_period, double t_pulsewidth, ExperimentNode_t * pNode)
{
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
  pNode->DCsamplingParams.DACMultEven = pNode->DCsamplingParams.DACMultOdd = 1;

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
    pNode->DCsamplingParams.DACMultOdd = bufMult;
    pNode->DCsamplingParams.DACMultEven = pNode->DCsamplingParams.DACMultOdd * (t_period - t_pulsewidth) / t_pulsewidth;
  }
  else
  {
    pNode->DCsamplingParams.DACMultEven = bufMult;
    pNode->DCsamplingParams.DACMultOdd = pNode->DCsamplingParams.DACMultEven * (t_pulsewidth / (t_period - t_pulsewidth));
  }
  pNode->DCsamplingParams.minorPointsDiscarded = isEvenPeriodShorter ? pNode->DCsamplingParams.DACMultEven / 2 : pNode->DCsamplingParams.DACMultOdd / 2;

  /* 3) Calculate ADCMult */
  pNode->DCsamplingParams.ADCBufferSizeEven = pNode->DCsamplingParams.DACMultEven;
  pNode->DCsamplingParams.ADCBufferSizeOdd = pNode->DCsamplingParams.DACMultOdd;

  pNode->DCsamplingParams.PointsSkippedPC = 1;
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

double ExperimentCalcHelperClass::calcNumberOfCycles(const ExperimentalAcData acDataHeader)
{
  double samplingFreq = 1.0e8 / (1 << acDataHeader.ADCacTimerDiv) / acDataHeader.ADCacTimerPeriod;
  freq_range_t freqRange = acDataHeader.frequency > HF_CUTOFF_VALUE ? HF_RANGE : LF_RANGE;
  double num_cycles;

  if (freqRange == HF_RANGE)
  {
    double n = acDataHeader.frequency / (acDataHeader.frequency - samplingFreq);
    num_cycles = ADCacBUF_SIZE / n;
  }
  else
  {
    num_cycles = ADCacBUF_SIZE / (samplingFreq / acDataHeader.frequency);
  }

  /* Account for unsynchronized ADC and signal gen clocks */
  if (acDataHeader.frequency >= 2.0e5)
    num_cycles *= 2;
  else if (acDataHeader.frequency >= 5.0e3)
    num_cycles *= 0.573345782 * log10(acDataHeader.frequency) - 1.192489631;
  else
    num_cycles *= 1;
  return num_cycles;
}

/* Sinusoidal curve fitting */
ComplexDataPoint_t ExperimentCalcHelperClass::AnalyzeFRA(double frequency, uint16_t * rawDataBuf, uint8_t numACBuffers, double gainEWE, double gainI, double approxNumCycles, const cal_t * calData, currentRange_t range)
{
  //todo: add error analysis, THD
  int newLen = numACBuffers * ADCacBUF_SIZE - ADCacROLLING_AVG_SIZE;
  double * t_data = new double[newLen];
  double * filteredCurrentData = new double[newLen];
  double * filteredEWEdata = new double[newLen];

  filterData(rawDataBuf, numACBuffers, filteredCurrentData, filteredEWEdata, ADCacROLLING_AVG_SIZE);

  for (int i = 0; i < newLen; i++)
    t_data[i] = i;

  /* Part 1: least squares regression first guess*/
  double resultsEWE[4];
  double resultsCurrent[4];
  sinusoidLeastSquaresFit(t_data, filteredCurrentData, newLen, resultsCurrent);
  sinusoidLeastSquaresFit(t_data, filteredEWEdata, newLen, resultsEWE);
  
  /* Part 2: Newton-Raphson method */  //TODO: don't just have this iterate 10 times, set an error limit
  double errorEWE = 0;
  double errorI = 0;
  double percentChange = 100;
  int numTries = 0;
  double fittedFreq = resultsEWE[0];
  while(percentChange > 0.01 && numTries < 20)
  {
    /* catch any guesses for frequency that are way off */
    double fittedNumCyclesEWE = ADCacBUF_SIZE * resultsEWE[0] / 2 / M_PI;
    double fittedNumCyclesCurrent = ADCacBUF_SIZE * resultsCurrent[0] / 2 / M_PI;
    bool freqErrorEWE = (fittedNumCyclesEWE / approxNumCycles > 1.25) || (fittedNumCyclesEWE / approxNumCycles < 0.75);
    bool freqErrorCurrent = (fittedNumCyclesCurrent / approxNumCycles > 1.25) || (fittedNumCyclesCurrent / approxNumCycles < 0.75);

    if (freqErrorEWE && !freqErrorCurrent)
      resultsEWE[0] = resultsCurrent[0];
    else if (!freqErrorEWE && freqErrorCurrent)
      resultsCurrent[0] = resultsEWE[0];
    else if (freqErrorEWE && freqErrorCurrent)
      resultsEWE[0] = resultsCurrent[0] = 2 * M_PI * approxNumCycles / ADCacBUF_SIZE;
    else
      resultsEWE[0] = resultsCurrent[0] = sqrt(resultsCurrent[0] * resultsEWE[0]);

    /* fit using the Newton-Raphson method */
    NewtonRaphson(resultsEWE, t_data, filteredEWEdata, newLen, resultsEWE);
    NewtonRaphson(resultsCurrent, t_data, filteredCurrentData, newLen, resultsCurrent);

    errorEWE = getError(filteredEWEdata, resultsEWE, newLen);
    errorI = getError(filteredCurrentData, resultsCurrent, newLen);

    double newFreq = (errorEWE > errorI) ? (resultsEWE[0] = resultsCurrent[0]) : (resultsCurrent[0] = resultsEWE[0]);
    percentChange = fabs(newFreq - fittedFreq) / fittedFreq * 100;
    fittedFreq = newFreq;
    numTries++;
  }

  /* debugging only */
  std::ofstream fout;
  QString filename = "C:/Users/mattc/Desktop/results";
  filename.append(QString::number(frequency));
  filename.append(".txt");
  fout.open(filename.toStdString(), std::ofstream::out);
  fout << "I fitted params:" << '\t' << resultsCurrent[0] << '\t' << resultsCurrent[1] << '\t' << resultsCurrent[2] << '\t' << resultsCurrent[3] << '\n';
  fout << "EWE fitted params:" << '\t' << resultsEWE[0] << '\t' << resultsEWE[1] << '\t' << resultsEWE[2] << '\t' << resultsEWE[3] << '\n';
  for (int i = 0; i < newLen; i++)
  {
    fout << filteredCurrentData[i] << '\t' << filteredEWEdata[i] << '\n';
  }

  ComplexDataPoint_t pt;
  double MagEWE = sqrt(pow(resultsEWE[2], 2) + pow(resultsEWE[3], 2)) / gainEWE;
  double MagCurrent = sqrt(pow(resultsCurrent[2], 2) + pow(resultsCurrent[3], 2)) / gainI;
  double phaseEWE = atan2(resultsEWE[2], resultsEWE[3]) * 180 / M_PI;
  double phaseCurrent = atan2(resultsCurrent[2], resultsCurrent[3]) * 180 / M_PI;
  pt.frequency = frequency;
  pt.ImpedanceMag = fabs(MagEWE / MagCurrent * (calData->m_DACdcP_I[range] / calData->m_DACdcP_V * 1000)); //yes, mIDACdc_I is in the numerator, since the units are inverted compared to m_ADC for I and V
  pt.phase = phaseCurrent - phaseEWE;
  if (fittedFreq < 0)
    pt.phase *= -1;
  if (pt.phase <= -180)
    pt.phase += 360;
  else if (pt.phase > 180)
    pt.phase -= 360;
  pt.ImpedanceReal = pt.ImpedanceMag * cos(pt.phase * M_PI / 180);
  pt.ImpedanceImag = pt.ImpedanceMag * sin(pt.phase * M_PI / 180);
  pt.error = getError(filteredEWEdata, resultsEWE, newLen) + getError(filteredCurrentData, resultsCurrent, newLen);

  delete[] t_data;
  delete[] filteredEWEdata;
  delete[] filteredCurrentData;

  return pt;
}

void ExperimentCalcHelperClass::sinusoidLeastSquaresFit(double * xbuf, double * ybuf, int size, double * results)
{
  double * S = new double[size];
  double * SS = new double[size];
  S[0] = SS[0] = 0;
  for (int i = 1; i < size; i++)
  {
    S[i] = S[i - 1] + 0.5*(ybuf[i] + ybuf[i - 1])*(xbuf[i] - xbuf[i - 1]);
    SS[i] = SS[i - 1] + 0.5*(S[i] + S[i - 1])*(xbuf[i] - xbuf[i - 1]);
  }

  double sum_x = 0, sum_x2 = 0, sum_x3 = 0, sum_x4 = 0, sum_SS = 0, sum_SS2 = 0, sum_xSS = 0, sum_x2SS = 0, sum_y = 0, sum_yx = 0, sum_yx2 = 0, sum_ySS = 0;

  for (int i = 0; i < size; i++)
  {
    sum_x += xbuf[i];
    sum_x2 += xbuf[i] * xbuf[i];
    sum_x3 += xbuf[i] * xbuf[i] * xbuf[i];
    sum_x4 += xbuf[i] * xbuf[i] * xbuf[i] * xbuf[i];
    sum_SS += SS[i];
    sum_SS2 += SS[i] * SS[i];
    sum_xSS += xbuf[i] * SS[i];
    sum_x2SS += xbuf[i] * xbuf[i] * SS[i];
    sum_y += ybuf[i];
    sum_yx += ybuf[i] * xbuf[i];
    sum_yx2 += ybuf[i] * xbuf[i] * xbuf[i];
    sum_ySS += ybuf[i] * SS[i];
  }

  double ** matrix1 = createMatrix(4, 4);
  double ** matrix2 = createMatrix(4, 1);

  /* Initialize matrix1 */
  matrix1[0][0] = sum_SS2;
  matrix1[0][1] = sum_x2SS;
  matrix1[0][2] = sum_xSS;
  matrix1[0][3] = sum_SS;

  matrix1[1][0] = sum_x2SS;
  matrix1[1][1] = sum_x4;
  matrix1[1][2] = sum_x3;
  matrix1[1][3] = sum_x2;

  matrix1[2][0] = sum_xSS;
  matrix1[2][1] = sum_x3;
  matrix1[2][2] = sum_x2;
  matrix1[2][3] = sum_x;

  matrix1[3][0] = sum_SS;
  matrix1[3][1] = sum_x2;
  matrix1[3][2] = sum_x;
  matrix1[3][3] = size;

  /* initialize matrix2 */
  matrix2[0][0] = sum_ySS;
  matrix2[1][0] = sum_yx2;
  matrix2[2][0] = sum_yx;
  matrix2[3][0] = sum_y;

  double ** invMatrix1 = invertMatrix(matrix1, 4);
  double ** matrixABCD = matrixMult(invMatrix1, 4, 4, matrix2, 4, 1);
  double A1 = matrixABCD[0][0];
  double B1 = matrixABCD[1][0];
  double C1 = matrixABCD[2][0];
  double D1 = matrixABCD[3][0];
  double w1 = sqrt(-A1);
  double a1 = 2 * B1 / w1;
  double b1 = (B1 * xbuf[0] * xbuf[0] + C1 * xbuf[0] + D1 - a1) * sin(w1 * xbuf[0]) + (C1 + 2 * B1 * xbuf[0]) / w1 * cos(w1 * xbuf[0]);
  double c1 = (B1 * xbuf[0] * xbuf[0] + C1 * xbuf[0] + D1 - a1) * cos(w1 * xbuf[0]) + (C1 + 2 * B1 * xbuf[0]) / w1 * sin(w1 * xbuf[0]);

  /* Part 2 */
  double a2 = a1;
  double p1 = sqrt(b1 * b1 + c1 * c1);
  double p2 = p1;
  double theta1 = 0;
  if (b1 > 0)
    theta1 = atan(c1 / b1);
  else if (b1 < 0)
    theta1 = atan(c1 / b1) + M_PI;
  else if (b1 == 0)
  {
    if (c1 > 0)
      theta1 = M_PI / 2;
    else
      theta1 = -M_PI / 2;
  }

  double * K = new double[size];
  for (int i = 0; i < size; i++)
  {
    K[i] = round((w1 * xbuf[i] + theta1) / M_PI);
  }

  double * Q = new double[size];
  for (int i = 0; i < size; i++)
  {
    if (p2*p2 > pow(ybuf[i] - a2, 2))
      Q[i] = pow(-1, K[i]) * atan((ybuf[i] - a2) / (p2*p2 - pow((ybuf[i] - a2), 2))) + M_PI * K[i];
    else
    {
      if (ybuf[i] > a2)
      {
        Q[i] = M_PI / 2 * pow(-1, K[i]) + M_PI * K[i];
      }
      else
      {
        Q[i] = -M_PI / 2 * pow(-1, K[i]) + M_PI * K[i];
      }
    }
  }

  double sum_Q = 0, sum_Qx = 0;
  for (int i = 0; i < size; i++)
  {
    sum_Q += Q[i];
    sum_Qx += Q[i] * xbuf[i];
  }
  double ** matrix3 = createMatrix(2, 2);
  matrix3[0][0] = sum_x2;
  matrix3[0][1] = sum_x;
  matrix3[1][0] = sum_x;
  matrix3[1][1] = size;
  double ** matrix4 = createMatrix(2, 1);
  matrix4[0][0] = sum_Qx;
  matrix4[1][0] = sum_Q;
  double ** invMatrix3 = invertMatrix(matrix3, 2);
  double ** matrixEF = matrixMult(invMatrix3, 2, 2, matrix4, 2, 1);
  double w2 = matrixEF[0][0];
  double theta2 = matrixEF[1][0];
  double b2 = p2 * cos(theta2);
  double c2 = p2 * sin(theta2);

  /* Part 3 */
  double w3 = w2;
  double sum_sin_w3x = 0, sum_cos_w3x = 0, sum_sin2_w3x = 0, sum_cos2_w3x = 0, sum_sincos_w3x = 0, sum_ysin_w3x = 0, sum_ycos_w3x = 0;
  for (int i = 0; i < size; i++)
  {
    double arg = w3 * xbuf[i];
    sum_sin_w3x += sin(arg);
    sum_cos_w3x += cos(arg);
    sum_sin2_w3x += pow(sin(arg), 2);
    sum_cos2_w3x += pow(cos(arg), 2);
    sum_sincos_w3x += sin(arg) * cos(arg);
    sum_ysin_w3x += ybuf[i] * sin(arg);
    sum_ycos_w3x += ybuf[i] * cos(arg);
  }

  double ** matrix5 = createMatrix(3, 3);
  matrix5[0][0] = size;
  matrix5[0][1] = sum_sin_w3x;
  matrix5[0][2] = sum_cos_w3x;
  matrix5[1][0] = sum_sin_w3x;
  matrix5[1][1] = sum_sin2_w3x;
  matrix5[1][2] = sum_sincos_w3x;
  matrix5[2][0] = sum_cos_w3x;
  matrix5[2][1] = sum_sincos_w3x;
  matrix5[2][2] = sum_cos2_w3x;

  double ** matrix6 = createMatrix(3, 1);
  matrix6[0][0] = sum_y;
  matrix6[1][0] = sum_ysin_w3x;
  matrix6[2][0] = sum_ycos_w3x;

  double ** invMatrix5 = invertMatrix(matrix5, 3);

  double ** matrixGHI = matrixMult(invMatrix5, 3, 3, matrix6, 3, 1);
  results[0] = w3;
  results[1] = matrixGHI[0][0];
  results[2] = matrixGHI[1][0];
  results[3] = matrixGHI[2][0];


  /* clean up */
  deleteMatrix(matrix1, 4);
  deleteMatrix(matrixABCD, 4);
  deleteMatrix(matrix2, 4);
  deleteMatrix(invMatrix1, 4);
  deleteMatrix(matrix3, 2);
  deleteMatrix(matrix4, 2);
  deleteMatrix(invMatrix3, 2);
  deleteMatrix(matrixEF, 2);
  deleteMatrix(matrix5, 3);
  deleteMatrix(matrix6, 3);
  deleteMatrix(invMatrix5, 3);
  deleteMatrix(matrixGHI, 3);
  delete[] S;
  delete[] SS;
  delete[] K;
  delete[] Q;
}

void ExperimentCalcHelperClass::filterData(uint16_t * rawData, uint8_t numACBuffers, double * smoothedIdataDest, double * smoothedEWEdataDest, int rollingAvgSize)
{
  int newLength = numACBuffers * ADCacBUF_SIZE - rollingAvgSize;
  double *separatedDataI = new double[numACBuffers * ADCacBUF_SIZE];
  double *separatedDataEWE = new double[numACBuffers * ADCacBUF_SIZE];
  //smoothedIdataDest = new double[newLength];
  //smoothedEWEdataDest = new double[newLength];
  

  for (int n = 0; n < numACBuffers; n++)
  {
      for (int i = 0; i < ADCacBUF_SIZE; i++)
      {
          separatedDataI[i + ADCacBUF_SIZE * n] = rawData[i + ADCacBUF_SIZE * n * 2];
          separatedDataEWE[i + ADCacBUF_SIZE * n] = rawData[i + ADCacBUF_SIZE * (2 * n + 1)];
      }
  }
  
  /* low-pass filter */
  for (int i = 0; i < newLength; i++)
  {
      smoothedIdataDest[i] = 0;
      smoothedEWEdataDest[i] = 0;
    for (int j = 0; j < rollingAvgSize; j++)
    {
        smoothedIdataDest[i] += separatedDataI[i + j];
        smoothedEWEdataDest[i] += separatedDataEWE[i + j];
    }
    smoothedIdataDest[i] /= rollingAvgSize;
    smoothedEWEdataDest[i] /= rollingAvgSize;
  }

  /* "high-pass filter" (remove average value) */
  double avgI = 0;
  double avgEWE = 0;
  for (int i = 0; i < newLength; i++)
  {
    avgI += smoothedIdataDest[i];
    avgEWE += smoothedEWEdataDest[i];
  }
  avgI /= newLength;
  avgEWE /= newLength;
  for (int i = 0; i < newLength; i++)
  {
      smoothedIdataDest[i] -= avgI;
      smoothedEWEdataDest[i] -= avgEWE;
  }

  delete[] separatedDataI;
  delete[] separatedDataEWE;
}

double ExperimentCalcHelperClass::getError(double * rawData, double * resultsBuf, int len)
{
  double sum = 0;
  for (int i = 0; i < len; i++)
  {
    sum += pow(rawData[i] - y_model(resultsBuf, i), 2);
  }
  return sqrt(sum);
}

double ** ExperimentCalcHelperClass::createMatrix(int rows, int cols)
{
  double ** matrix = new double*[rows];
  for (int i = 0; i < rows; i++)
  {
    matrix[i] = new double[cols];
  }

  return matrix;
}
void ExperimentCalcHelperClass::deleteMatrix(double ** matrix, int rows)
{
  for (int i = 0; i < rows; i++)
  {
    delete[] matrix[i];
  }
  delete[] matrix;
}
double ** ExperimentCalcHelperClass::invertMatrix(double ** matrix, int size)
{

  /* Get matrix of minors */
  double ** matrixOfMinors = createMatrix(size, size);
  for (int r = 0; r < size; r++)
  {
    for (int c = 0; c < size; c++)
    {
      double ** minorMatrix = getMinorMatrix(matrix, r, c, size);
      matrixOfMinors[r][c] = determinant(minorMatrix, size - 1);
      deleteMatrix(minorMatrix, size - 1);
    }
  }

  /* matrix of cofactors */
  for (int r = 0; r < size; r++)
  {
    for (int c = 0; c < size; c++)
    {
      matrixOfMinors[r][c] *= ((c % 2) == 0 ? 1 : -1) * ((r % 2) == 0 ? 1 : -1);
    }
  }

  /* get Adjugate */
  double ** inverseMatrix = createMatrix(size, size);
  double determinant_ = determinant(matrix, size);
  for (int r = 0; r < size; r++)
  {
    for (int c = 0; c < size; c++)
    {
      inverseMatrix[c][r] = matrixOfMinors[r][c];
      inverseMatrix[c][r] /= determinant_;
    }
  }


  deleteMatrix(matrixOfMinors, size);
  return inverseMatrix;
}
double ExperimentCalcHelperClass::determinant(double ** matrix, int size)
{
  if (size == 1)
    return matrix[0][0];
  else
  {
    double sum = 0;
    for (int i = 0; i < size; i++)
    {
      double ** minorMatrix = getMinorMatrix(matrix, 0, i, size);
      double term = matrix[0][i] * determinant(minorMatrix, size - 1);
      term *= (i % 2) == 0 ? 1 : -1;
      sum += term;
      deleteMatrix(minorMatrix, size - 1);
    }
    return sum;
  }
}
double ** ExperimentCalcHelperClass::getMinorMatrix(double ** matrix, int row, int col, int size)
{
  double ** minorMatrix = createMatrix(size - 1, size - 1);
  for (int c = 0, srcCol = 0; c < size - 1; c++, srcCol++)
  {
    if (c == col)
      srcCol++;
    for (int r = 0, srcRow = 0; r < size - 1; r++, srcRow++)
    {
      if (r == row)
        srcRow++;
      minorMatrix[r][c] = matrix[srcRow][srcCol];
    }
  }
  return minorMatrix;
}
double ** ExperimentCalcHelperClass::matrixMult(double ** matrix1, int rows1, int cols1, double ** matrix2, int rows2, int cols2)
{
  if (cols1 != rows2)
    return NULL;

  double ** result = createMatrix(rows1, cols2);
  for (int r = 0; r < rows1; r++)
  {
    for (int c = 0; c < cols2; c++)
    {
      double sum = 0;
      for (int i = 0; i < cols1; i++)
        sum += matrix1[r][i] * matrix2[i][c];
      result[r][c] = sum;
    }
  }
  return result;
}

void ExperimentCalcHelperClass::NewtonRaphson(double * initialGuessParams, double * xbuf, double * ybuf, int length, double * resultsBuf, bool lockedFrequency)
{
  if (lockedFrequency == false)
  {
    double ** doubleDerivMatrix = createMatrix(4, 4);

    double(*ptrFnDblDerivs[4][4])(double *, double);
    ptrFnDblDerivs[0][0] = d2ydw2;
    ptrFnDblDerivs[0][1] = d2ydwda;
    ptrFnDblDerivs[0][2] = d2ydwdb;
    ptrFnDblDerivs[0][3] = d2ydwdc;
    ptrFnDblDerivs[1][0] = d2ydwda;
    ptrFnDblDerivs[1][1] = d2yda2;
    ptrFnDblDerivs[1][2] = d2ydadb;
    ptrFnDblDerivs[1][3] = d2ydadc;
    ptrFnDblDerivs[2][0] = d2ydwdb;
    ptrFnDblDerivs[2][1] = d2ydadb;
    ptrFnDblDerivs[2][2] = d2ydb2;
    ptrFnDblDerivs[2][3] = d2ydbdc;
    ptrFnDblDerivs[3][0] = d2ydwdc;
    ptrFnDblDerivs[3][1] = d2ydadc;
    ptrFnDblDerivs[3][2] = d2ydbdc;
    ptrFnDblDerivs[3][3] = d2ydc2;

    double(*ptrFnDerivs[4])(double *, double);
    ptrFnDerivs[0] = dydw;
    ptrFnDerivs[1] = dyda;
    ptrFnDerivs[2] = dydb;
    ptrFnDerivs[3] = dydc;

    for (int r = 0; r < 4; r++)
    {
      for (int c = 0; c < 4; c++)
      {
        doubleDerivMatrix[r][c] = de2dXdY(initialGuessParams, xbuf, ybuf, length, ptrFnDerivs[r], ptrFnDerivs[c], ptrFnDblDerivs[r][c]);
      }
    }

    double ** singleDerivMatrix = createMatrix(4, 1);
    for (int r = 0; r < 4; r++)
    {
      singleDerivMatrix[r][0] = dedX(initialGuessParams, xbuf, ybuf, length, ptrFnDerivs[r]);
    }

    double ** dblDerivMatrInverse = invertMatrix(doubleDerivMatrix, 4);
    double ** results = matrixMult(dblDerivMatrInverse, 4, 4, singleDerivMatrix, 4, 1);
    for (int i = 0; i < 4; i++)
    {
      resultsBuf[i] = initialGuessParams[i] - results[i][0];
    }

    deleteMatrix(dblDerivMatrInverse, 4);
    deleteMatrix(doubleDerivMatrix, 4);
    deleteMatrix(singleDerivMatrix, 4);
    deleteMatrix(results, 4);
  }
  else
  {
    double ** doubleDerivMatrix = createMatrix(3, 3);

    double(*ptrFnDblDerivs[3][3])(double *, double);
    ptrFnDblDerivs[0][0] = d2yda2;
    ptrFnDblDerivs[0][1] = d2ydadb;
    ptrFnDblDerivs[0][2] = d2ydadc;
    ptrFnDblDerivs[1][0] = d2ydadb;
    ptrFnDblDerivs[1][1] = d2ydb2;
    ptrFnDblDerivs[1][2] = d2ydbdc;
    ptrFnDblDerivs[2][0] = d2ydadc;
    ptrFnDblDerivs[2][1] = d2ydbdc;
    ptrFnDblDerivs[2][2] = d2ydc2;

    double(*ptrFnDerivs[3])(double *, double);
    ptrFnDerivs[0] = dyda;
    ptrFnDerivs[1] = dydb;
    ptrFnDerivs[2] = dydc;

    for (int r = 0; r < 3; r++)
    {
      for (int c = 0; c < 3; c++)
      {
        doubleDerivMatrix[r][c] = de2dXdY(initialGuessParams, xbuf, ybuf, length, ptrFnDerivs[r], ptrFnDerivs[c], ptrFnDblDerivs[r][c]);
      }
    }

    double ** singleDerivMatrix = createMatrix(3, 1);
    for (int r = 0; r < 3; r++)
    {
      singleDerivMatrix[r][0] = dedX(initialGuessParams, xbuf, ybuf, length, ptrFnDerivs[r]);
    }

    double ** dblDerivMatrInverse = invertMatrix(doubleDerivMatrix, 3);
    double ** results = matrixMult(dblDerivMatrInverse, 3, 3, singleDerivMatrix, 3, 1);
    for (int i = 0; i < 3; i++)
    {
      resultsBuf[i + 1] = initialGuessParams[i + 1] - results[i][0];
    }

    deleteMatrix(dblDerivMatrInverse, 3);
    deleteMatrix(doubleDerivMatrix, 3);
    deleteMatrix(singleDerivMatrix, 3);
    deleteMatrix(results, 3);
  }
}

double ExperimentCalcHelperClass::y_model(double * paramsBuf, double x)
{
  double w = paramsBuf[0];
  double a = paramsBuf[1];
  double b = paramsBuf[2];
  double c = paramsBuf[3];
  return a + b*sin(w*x) + c*cos(w*x);
}
double ExperimentCalcHelperClass::dedX(double * paramsBuf, double * xbuf, double * ybuf, int size, double(*dydX)(double *, double))
{
  double sum = 0;
  for (int i = 0; i < size; i++)
  {
    sum += 2 * (y_model(paramsBuf, xbuf[i]) - ybuf[i]) * dydX(paramsBuf, xbuf[i]);
  }
  return sum;
}
double ExperimentCalcHelperClass::de2dXdY(double * paramsBuf, double * xbuf, double * ybuf, int size,
  double(*dydX)(double *, double),
  double(*dydY)(double *, double),
  double(*d2ydXdY)(double *, double))
{
  double sum = 0;
  for (int i = 0; i < size; i++)
  {
    sum += 2 * (y_model(paramsBuf, xbuf[i]) - ybuf[i])*d2ydXdY(paramsBuf, xbuf[i]) + 2 * dydX(paramsBuf, xbuf[i])*dydY(paramsBuf, xbuf[i]);
  }
  return sum;
}
double ExperimentCalcHelperClass::dydw(double * paramsBuf, double x)
{
  double w = paramsBuf[0];
  double b = paramsBuf[2];
  double c = paramsBuf[3];
  return x*(b*cos(w*x) - c*sin(w*x));
}
double ExperimentCalcHelperClass::dyda(double * paramsBuf, double x)
{
  return 1;
}
double ExperimentCalcHelperClass::dydb(double * paramsBuf, double x)
{
  double w = paramsBuf[0];
  return sin(w*x);
}
double ExperimentCalcHelperClass::dydc(double * paramsBuf, double x)
{
  double w = paramsBuf[0];
  return cos(w*x);
}
double ExperimentCalcHelperClass::d2ydw2(double * paramsBuf, double x)
{
  double w = paramsBuf[0];
  double b = paramsBuf[2];
  double c = paramsBuf[3];
  return -x*x*b*sin(w*x) - x*x*c*cos(w*x);
}
double ExperimentCalcHelperClass::d2ydwda(double * paramsBuf, double x)
{
  return 0;
}
double ExperimentCalcHelperClass::d2ydwdb(double * paramsBuf, double x)
{
  double w = paramsBuf[0];
  return x*cos(w*x);
}
double ExperimentCalcHelperClass::d2ydwdc(double * paramsBuf, double x)
{
  double w = paramsBuf[0];
  return -x*sin(w*x);
}
double ExperimentCalcHelperClass::d2yda2(double * paramsBuf, double x)
{
  return 0;
}
double ExperimentCalcHelperClass::d2ydadb(double * paramsBuf, double x)
{
  return 0;
}
double ExperimentCalcHelperClass::d2ydadc(double * paramsBuf, double x)
{
  return 0;
}
double ExperimentCalcHelperClass::d2ydb2(double * paramsBuf, double x)
{
  return 0;
}
double ExperimentCalcHelperClass::d2ydbdc(double * paramsBuf, double x)
{
  return 0;
}
double ExperimentCalcHelperClass::d2ydc2(double * paramsBuf, double x)
{
  return 0;
}

//=============================================================================
namespace // unnamed namespace to make these function definitions private within this file
{
    // func Average: Computes the average of a vector.
    //
    // in     x: 
    // returns: The average of x.
    int Average(std::vector<int> const& x)
    {
        if (x.size() < 1) { return 0; }

        int sum = 0;
        for (int i = 0; i < x.size(); i++)
        {
            sum += x[i];
        }
        int average = sum / x.size();
        return average;
    }

    // func Zero: Computes a "zero-value" of a vector.
    //
    // in     x: 
    // returns: The zero-value of x.
    int Zero(std::vector<int> const& x)
    {
        // CHOOSE: find the zero by taking the average
        int zero = Average(x);

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
    void AllZeroCrossings(std::vector<int>& zca, std::vector<int> const& x, int zero)
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
    int Edge1(std::vector<int> const& zc)
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
    void ZeroCrossIntervals(std::vector< std::pair<int, int> >& zci, std::vector<int> const& zca, int nx)
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
            std::pair<int, int> interval(beg, end);
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
    
        std::pair<int, int> interval(beg, end);
        zci.push_back(interval);

        std::cout << "zci: ";
        for (int i = 0; i < zci.size(); i++) {
            std::cout << zci[i].first << ":" << zci[i].second << ",";
        }
        std::cout << std::endl;
    }

    int AverageDistance(std::vector< std::pair<int, int> >& x)
    {
        int average_distance = 0;
        for (int i = 0; i < x.size(); i++)
        {
            int dist = x[i].second - x[i].first;
            average_distance += dist / (i + 1);
        }
        return average_distance;
    }

    int AverageDistance(std::vector<int> const& x)
    {
        // backward-difference
        int average_distance = 0;
        for (int i = 1; i <= x.size(); i++)
        {
            average_distance += (x[i] - x[i - 1]) / i;
        }
        return average_distance;
    }

    void SmoothZeroCrossings(std::vector<int>& zca, std::vector< std::pair<int, int> > const& zci)
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
int ExperimentCalcHelperClass::GetFrequency(double const *const xbuf_smoothed, int const size)
{

    // Algorithm Variables
    std::vector<int> x(xbuf_smoothed, xbuf_smoothed + size * sizeof(xbuf_smoothed[0])); // copy xbuf array into x vector
    std::vector<int> zca; // indicies of all zero-crossings
    std::vector< std::pair<int, int> > zci; // index-pairs of zero-crossing intervals
    int zero = 0; // computed "zero-value"

    // choose a zero-value
    zero = Zero(x);

    // find all zero-crossings
    AllZeroCrossings(zca, x, zero);

    // find zero-crossing intervals
    ZeroCrossIntervals(zci, zca, x.size());

    // overwrite zca with clusters resolved to single points
    zca.clear();
    SmoothZeroCrossings(zca, zci);

    int frequency = AverageDistance(zca);
    return frequency;
}


