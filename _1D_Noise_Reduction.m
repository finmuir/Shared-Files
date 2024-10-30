% Import clean speech signal
[speech_clean,fs] = audioread('MT1_speech_clean.wav');
% Calculate power of the signal 
power = mean(speech_clean.^2);

% Calculate power in dB
power_in_db = 10 * log10(power);

% Add Additive Gausian White Noise to the speech clean signal to form
% speech_noisy
rng(1); % sets the random signal generator
speech_noisy = awgn(speech_clean,2,power_in_db); %2dB white noise added

%plot noisy and clean signals

figure, 
plot(speech_clean);
plot(speech_noisy);

hold on



% Reduce noise in speech_noisy by convolving it with the hn1 filter to form
% speech_out
hn1 = 1/9 * [ones(1,9)];
speech_out = conv(speech_noisy,hn1,'same');

% Investigate frequency characteristics of the hn1 smoothing filter
figure(2);
freqz(hn1);
h = freqz(hn1);

figure(3);
plot(h);

%Low noise
rng(1); % sets the random signal generator

% Calcualte snr_out for Low noise, snr  = 10
xin_low = awgn(speech_clean,10,power_in_db);
snr_in_low = 10*log10(power/10);

xout_low = conv(xin_low,hn1,"same");
sigout = conv(speech_clean,hn1,"same");

residual_noise_low = xout_low - sigout;
power_residual_noise_low = mean(residual_noise_low.^2);
p_xout_low = mean(xout_low.^2);

snr_out_low = 10*log10(p_xout_low/(power_residual_noise_low))



%Moderate noise
rng(1); % sets the random signal generator

% Calcualte snr_out for MODERATE noise, snr  = 5
xin_moderate = awgn(speech_clean,5,power_in_db);
snr_in_moderate = 10*log10(power/10);

xout_moderate = conv(xin_moderate,hn1,"same");
%sigout = conv(speech_clean,hn1,"same");

residual_noise_moderate = xout_moderate - sigout;
power_residual_noise_moderate = mean(residual_noise_moderate.^2);
p_xout_moderate = mean(xout_moderate.^2);

snr_out_moderate = 10*log10(p_xout_moderate/(power_residual_noise_moderate))



%High noise
rng(1); % sets the random signal generator

xin_high = awgn(speech_clean,-5,power_in_db);
snr_in_high = 10*log10(power/10);

xout_high = conv(xin_high,hn1,"same");
%sigout = conv(speech_clean,hn1,"same");

residual_noise_high = xout_high - sigout;
power_residual_noise_high = mean(residual_noise_high.^2);
p_xout_high = mean(xout_high.^2);

snr_out_high = 10*log10(p_xout_high/(power_residual_noise_high))
