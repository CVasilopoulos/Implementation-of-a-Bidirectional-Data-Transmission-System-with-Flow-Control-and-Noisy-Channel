%%
%      ������������ ������������� ���������
%
%      ����������� 2015
%
%      ������ GenSink_RS232_HFC_1P.m
%
%       ��������� �����������, ����������� ��� ������� ���������� ���� RS232 port 
%       �� ������ ����.  � ���������� ����� ������������ ����������������.
%       �� ���������� ��� ������������ ������������� ��� array  TxChars.
%       O� ���������� ��� ����������� ������������� ��� array  RxChars.
%
%       ��� Command Window  ��������� �������� ����������� ��� ��� ���������  ��� ����������
%       ��� ����� ������� �� ������� ������ ��� � ��������� ������/������ ���������.
%
%       ��� �� ������� ��� ��������� ���  ������������, �������    Ctrl-C
%       ��� ���� ������ �� ���������� �  ������  fclose(s1); clear s1;  ��� Command Window .
%
%
clear all;
close all;
clc;

disp('Starting the Characters Gen/Sink Application . . . . ');

Nc = 1000;           %  ������ ���������� ���� ��������/����
Nstart = 0;         %  ������� ����� ���������� ���� ��������
Nend = 255;
pr_off = 0.3;       %  ���������� ��������������� �����

disptime = 2;       %  ������ ���������� (secs)
Nbuffer = 2048;      % Tx/Rx buffer length

TxChars = uint8(randi([Nstart Nend], 1, Nc));
% TxChars = uint8(randi([20 20], 1, Nc));
RxChars = uint8(zeros(1, Nc));

%%
disp('Opening the RS232 port . . . . . ');
s1 = serial('COM6','BaudRate',9600,'Parity','none', 'Terminator', '');
set(s1, 'FlowControl', 'none');
set(s1, 'InputBufferSize', Nbuffer);
set(s1, 'OutputBufferSize', Nbuffer);
fopen(s1)
s1.RecordDetail = 'verbose';
s1.RecordName = 'S1.txt';
record(s1,'on')
disp('RS232 port activated');
disp(' ');


%%
tstart = clock;
tinit = tstart;

cnt_t = 0;
cnt_r = 0;
while cnt_t < Nc || cnt_r < Nc
    arv = s1.PinStatus.ClearToSend;      %  Check if Sink is ON  and the output buffer empty
    if length(arv) == 2  && s1.BytesToOutput < Nbuffer && cnt_t < Nc
        cnt_t = cnt_t + 1;          % generate new character
        fwrite(s1,TxChars(cnt_t));
    end
    
    %  Deactivate or activate the receiver
    %  Deactivate for 1 sec the receiver with probability pr_off
   deactpr = rand(1,1);
   if deactpr <= pr_off
        s1.RequestToSend = 'off';        %  Disable DTR (Rx OFF)
        pause(3*rand(1,1));
        s1.RequestToSend = 'on';         %  Enable DTR (Rx ON)
   end   
    
    
%     deactpr = rand(1,1);
%     if deactpr <= pr_off
%         disp('OFF'); %pause(0.01);
%         s1.RequestToSend = 'off';        %  Disable RTS (Rx OFF)
%          disp('ON');
%     else
        s1.RequestToSend = 'on';         %  Enable RTS (Rx ON)
        len = s1.BytesAvailable;    % wait to receive a new character
        if len > 0   
            RxChars(cnt_r+ 1:cnt_r+len) = fread(s1,len);
            cnt_r=cnt_r+len;
        end
%     end 

%     timedif = etime(clock, tstart);         %  Update results
%     if timedif > disptime
%         fprintf('\n  Outgoing Characters: %d  of %d  ', cnt_t, Nc);
%         fprintf('\n  Incoming Characters: %d  of %d \n', cnt_r, Nc);
%         tstart = clock;
%     end

end

timedif = etime(clock, tinit);      %  Total time

%%
disp(' ');
disp('Closing the RS232 port . . . . . ');
record(s1,'off')
fclose(s1)
delete(s1)
clear s1
disp('RS232 port deactivated');

disp(' ');
if TxChars == RxChars
    disp('Correct Data Transmission');
else
    disp('Incorrect Data Transmission');
  
    fprintf('\nCharacter Error Ratio =  %6.5f', sum(TxChars ~= RxChars)/Nc);
end




fprintf('\nCharacters exchanged: %d\nTime elapsed: %4.3f secs\nData rate: %4.3f Bps  \n\n', Nc, timedif, Nc/timedif);

disp(' . . . .  Ending the Characters Gen/Sink Application.');
