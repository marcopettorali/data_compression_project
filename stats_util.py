import math
from scipy.stats import norm

confidence = 0.95


def s_mean_value(samples):
    mean = 0
    for sample in samples:
        mean += sample
    mean /= len(samples)
    return mean


def s_variance(samples):
    mean = s_mean_value(samples)
    variance = 0
    for sample in samples:
        variance += (sample - mean) ** 2
    variance /= (len(samples) - 1)
    return variance



def normal_dist_percentile(p):
    percentile = norm.ppf(p)
    return percentile

def s_mean_confidence_interval(samples):
    mean = s_mean_value(samples)
    variance = s_variance(samples)
    percentile = normal_dist_percentile(1-(1-confidence)/2)
    radius = percentile * math.sqrt(variance/len(samples))
    return [mean-radius, mean+radius]
