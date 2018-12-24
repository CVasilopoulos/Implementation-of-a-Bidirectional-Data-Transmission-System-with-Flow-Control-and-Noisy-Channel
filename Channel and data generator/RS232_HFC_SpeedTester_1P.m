%%
%  Ενσωματωμένα Επικοινωνιακά Συστήματα
%
%   Δεκέμβριος 2015
%
%   Αρχείο RS232_HFC_SpeedTester_1P.m
%
%   RS232 communications Speed Tester  (RS232 uses HFC)
%
clear all;
close all;
clc;

disp('Opening the RS232 port . . . . . ');
sport = serial('COM6','BaudRate', 9600, 'Parity','none', 'Terminator', '');
fopen(sport)
disp('RS232 port activated');
disp(' ');

disp('Transmission Speed Tester');

Ns = 10000;              % No of characters to transmit
Cmin = 0;               % minimum character value
Cmax = 255;             % maximum character value

data_out = uint8(randi([Cmin Cmax], 1, Ns));
%data_out = uint8(randi([1 1], 1, Ns));
data_in = uint8(zeros(1,Ns));

tx_cnt = 0;
rx_cnt = 0;
ts = tic;
while (tx_cnt < Ns || rx_cnt < Ns)
    if (tx_cnt < Ns) && (length(sport.PinStatus.ClearToSend) == 2) ...
            && (sport.BytesToOutput == 0)   % CTS = 1
        tx_cnt = tx_cnt+ 1;
        fwrite(sport, data_out(tx_cnt), 'uchar');  
    end
    if rx_cnt < Ns
        b2r = sport.BytesAvailable;
        if b2r > 0
            data_in(rx_cnt + 1:rx_cnt + b2r) = fread(sport,b2r);
            rx_cnt = rx_cnt + b2r;
        end
    end
end
time_elapsed = toc(ts);

%%
if data_out == data_in
    disp('Data Exchanged Correctly!')
    fprintf('\nData Rate = %4.2f bytes/sec\n\n', Ns/time_elapsed);
else
    ch_e = (data_out- uint8(data_in))~=0;
    disp('Error During Data Exchange!!!!')
    fprintf('\nProbability of error = %5.4f\n', sum(ch_e)/Ns);
    subplot(2,1,1);
    plot(data_in,'ok'); hold on; plot(data_out,'ob');  plot(abs(data_out - data_in), '*r');
    subplot(2,1,2);
    plot(ch_e);
end


%%
disp('Closing the RS232 port . . . . . ');
fclose(sport)
delete(sport)
clear sport
disp('RS232 port deactivated');
disp(' ');
