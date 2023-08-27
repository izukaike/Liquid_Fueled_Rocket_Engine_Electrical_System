%{
   Test Software

   Meant to be quick way to test Arduino software and Hardware
   the set up is to send and recieve data from arduino that simulates
   real time environments for controlled environments.

   More about the structure of the code. I want the code to be setup in a way that each test is its own independent
   function so someone can look at this file and test whatever they need as fast possible with little dpeendencies.
   Look at my example below where I have the function definition and extremeley detailed function and a simple descriptions
   so anyone looking can follow along.
   
   Created: 8/26/23 
   Contributors: 
   - Izuka Ikedionwu
   - *if your line of code is added/deleted or modified add name here*
%}
%%Main Code Section 





%%Arduino Set Up

%%----------------------

%{
   used for testing layouts and data readng for prelim software
   Engineer: Izuka Ikedionwu
%}
function [retVal] = PhotoResistor2_A0_A5_Setup_Test()
a = arduino();
%add validation check
pin_A0 = ('A0');
pin_A5 = ('A5');
forever = 0;
j = 1;
%adjustable window
x_low   = -4;
x_high = 2;
y_low  = 0;
y_high = 5;

while forever < 1
        %reading voltage------ code
        A0(j) = a.readVoltage(pin_A0); 
        A5(j) = a.readVoltage(pin_A5); 
        %----------------------------
        %Subplot code
        subplot(1,2,1)
        plot(A0(:),'-')
        subplot(1,2,2)
        plot(A5(:),'-')
        subplot(1,2,1)
        xlim([x_low x_high])
        ylim([y_low y_high])
        subplot(1,2,2)
        xlim([x_low x_high])
        ylim([y_low y_high])
        j = j+1; 
        x_low = x_low + 1;
        x_high = x_high + 1;
        %--------------------------
        %file handling section but it depends on the test
end
retVal = 1
end


