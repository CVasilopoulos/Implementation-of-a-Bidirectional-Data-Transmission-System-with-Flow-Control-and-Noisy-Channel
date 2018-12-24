%%
%      Ενσωματωμένα Επικοινωνιακά Συστήματα
%      
%      Φεβρουάριος 2015
%
%      Αρχείο Channel_Noise_Phase.m
%
%      Λαμβάνει χαρακτήρες από το ένα COM-x και τους στέλνει σε άλλο port COM-y, 
%      αφού πρώτα προστεθεί θόρυβος και απόκλιση φάσης στο σήμα. 
%      Απεικονίζει τα δείγματα σε 'παλμογραφο' και αναλυτή φάσματος.
%
%      Επιλογές:
%      - Λειτουργία σε πεπερασμένο πλήθος δειγμάτων (παράμετρος maxNS) 
%      - Εισαγωγή θορύβου με συγκεκριμένη ισχύ (παράμετρος Pnoise). 
%      - Εισαγωγή διαφοράς φάσης  με γραμμική παρεμβολή  (παράμετρος phOffset)
%
%
%      Για τη διακοπή της εκτέλεσης του  προγράμματος, πατήστε    Cntrl-C    
%      και μετα πρέπει να εκτελεστεί το τελευταίο τμήμα του κώδικα για την απενεργοποίηση των RS232 ports.
%
%      Στο Command Window  περιοδικά δίνονται πληροφορίες για την κατάσταση  του συστήματος
%
close all;
clear all;
clc;

    
    maxNS = 38400;            % Maximum number of samples 
    disptime = 5;            %  Display update time  (secs)
    
    Rsam = 8000;             % Sample rate [sps]
    Pnoise = 20/1000;         %  Watt,    (No noise = 0)
    phOffset = 0.00;         %   Phase offset  (-1 <   < 1,      -1 = x_{+1},   0 = x_{0},   1 = x_{-1})     

    
    scrsz = get(0,'ScreenSize');
    
%  Display parameters
    Ndisp = 1024;         %   update parameters
    Nfft = 512;                   %  FFT points
    Nmax = max(Nfft, Ndisp);
    
%  DAC/ADC
    Bdac = 8;
    DRdac = 2;               %  DAC dynamic range,  in Volts
    dac_co_ain = DRdac/(2^Bdac -1);
    dac_co_bin = -DRdac/2;
    dac_co_aout = (2^Bdac -1)/DRdac;
    dac_co_bout = (2^Bdac -1)/2;
    
    if Pnoise > 0
        sgma = sqrt(Pnoise);
    end
    
    Farrow_taps_AB = [0 0 0 0];        
    Farrow_taps_BA = [0 0 0 0]; 

%%      Data Processing
disp(' Processing serial data .....');
fprintf('\nChannel emulator parameters:\n-----------------------------------------');

if maxNS == Inf
    fprintf('\nContinuous Operation');
else
    fprintf('\nSamples to be processed = %d', maxNS);
end

if Pnoise == 0
    fprintf('\nNo noise is introduced');
else
    fprintf('\nNoise power = %6.3f mWatts', Pnoise*1000);
end


if phOffset == 0
    fprintf('\nNo phase offset is introduced');
else
    fprintf('\nIntroduced phase offset = %d%% of the input sample period', phOffset*100);
end

disp(' ');

    cnt_in = 0;
    cnt_out = 0;
    mu = -abs(phOffset);              %  Initial phase
    
%%   Serial Ports Initialization
    disp('Serial ports initialization.....');
    rs_rate = 38400;
    RxBufferSize = 131072;
    TxBufferSize = 131072;
    
    sport_A = serial('COM7', 'BaudRate', rs_rate, 'Parity','none', 'Terminator', '','InputBufferSize', RxBufferSize,'OutputBufferSize', TxBufferSize);
    fopen(sport_A)   

    sport_B = serial('COM11', 'BaudRate', rs_rate, 'Parity','none', 'Terminator', '','InputBufferSize', RxBufferSize,'OutputBufferSize', TxBufferSize);
    fopen(sport_B)   
    
    
%%
noise = 12; % 12 dB
rx_cnt = 0;
while (1)
    % A-B
    len = sport_A.BytesAvailable;
    if len > 0
        InChar = fread(sport_A,len);
        Vin = dac_co_ain*InChar + dac_co_bin;              %  Voltage
%         Vout = awgn(Vin,noise,'measured');
        Vout = Vin;
        Vout = min(DRdac/2, Vout);                         %  Limiter
        Vout = max(-DRdac/2, Vout);
        OutChar = dac_co_aout*Vout + dac_co_bout;
        fwrite(sport_B, OutChar, 'uchar');
        cnt_in = cnt_in + len;
    end

%     % B-A (ACK PACKETs)
     len = sport_B.BytesAvailable;
     if len > 0
        InChar = fread(sport_B,len);
        Vin = dac_co_ain*InChar + dac_co_bin;              %  Voltage
%         Vout = awgn(Vin,noise,'measured');
        Vout = Vin;
        Vout = min(DRdac/2, Vout);                         %  Limiter
        Vout = max(-DRdac/2, Vout);
        OutChar = dac_co_aout*Vout + dac_co_bout;
        fwrite(sport_A, OutChar, 'uchar');
        cnt_out = cnt_out + len;
     end
end
               

%%    End of Session
disp('Closing Serial ports .....');
fclose(sport_A)
clear sport_A
fclose(sport_B)
clear sport_B
disp('End of Session');

