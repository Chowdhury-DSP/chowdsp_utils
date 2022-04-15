# %%
import numpy as np
from scipy.io import wavfile
import audio_dspy as adsp
import matplotlib.pyplot as plt

# %%
fs, x = wavfile.read('spring-reverb-impulse.wav')
x = x[2325:200000] / 2**15
# sample rate: 44100

# %%
plt.plot(x)

# %%
# needs a few more high freq. modes, any maybe try to get the total # of modes below 125?
freqs, peaks = adsp.find_freqs(x, fs, above=30, thresh=0, frac_off=0.01, plot=True)
plt.xlim(20, 20000)
print(len(freqs))

# %%
taus = np.zeros_like(freqs)

# %%
taus[:5] = adsp.find_decay_rates(freqs[:5], x[:int(fs*2.0)], fs, 30, thresh=-18, plot=False)

# %%
taus[5:60] = adsp.find_decay_rates(freqs[5:60], x[:int(fs*2.5)], fs, 30, thresh=-22, plot=False)


# %%
taus[60:] = adsp.find_decay_rates(freqs[60:], x[:int(fs*3.5)], fs, 30, thresh=-20, plot=False)

# %%
# taus[40:50] = adsp.find_decay_rates(freqs[40:50], x[:int(fs*2.5)], fs, 30, thresh=-22, plot=True)

# %%
amps = adsp.find_complex_amplitudes (freqs, taus, int(len(x) * 0.55), x, fs)

# %%
y = adsp.generate_modal_signal(amps, freqs, taus, len(amps), len(x), fs)

# %%
plt.plot(y * np.max(np.abs(x)))
plt.plot(x)

# %%
X = adsp.normalize(np.fft.rfft (x))
Y = adsp.normalize(np.fft.rfft (y))
f = np.linspace (0, fs/2, num=len(X))
plt.semilogx (f, 20 * np.log10 (np.abs (X)))
plt.semilogx (f, 20 * np.log10 (np.abs (Y)))
plt.xlim(20, 20000)

# %%
y_wav = (y * 2**15).astype(np.int16)
wavfile.write('impulse-render.wav', fs, y_wav)

# %%
print(freqs)

# %%
print(taus)

# %%
print(amps.real)

# %%
print(amps.imag)

# %%
