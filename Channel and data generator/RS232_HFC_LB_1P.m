%%
%  Ενσωματωμένα Επικοινωνιακά Συστήματα
%
%   Δεκέμβριος 2015
%
%   Αρχείο RS232_HFC_LB_1P.m
%
%   RS232 Loopback  (RS232 uses HFC)
%
clear all;
close all;
clc;

disp('Opening the RS232 port . . . . . ');
LB = 131072;
sport = serial('COM14','BaudRate', 9600, 'Parity','none', 'Terminator', '');
set(sport, 'FlowControl', 'none');
set(sport, 'InputBufferSize', LB);
set(sport, 'OutputBufferSize',LB);
fopen(sport)
disp('RS232 port activated');
disp(' ');
sport.RequestToSend = 'off';
disp('RS232 Loop-back mode');
sport.RequestToSend = 'on';
while (1)
    if length(sport.PinStatus.ClearToSend) == 2
        sport.RequestToSend = 'on';
        if sport.BytesToOutput < LB
            b2r = sport.BytesAvailable;
            if b2r > 0
                data_val = fread(sport,1);
                fwrite(sport, data_val, 'uchar');
            end
        end
    else
        sport.RequestToSend = 'off';
    end
end

%%
disp('Closing the RS232 port . . . . . ');
fclose(sport)
delete(sport)
clear sport
disp('RS232 port deactivated');
disp(' ');