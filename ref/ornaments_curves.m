close all
clear
clc

N = 1;
n = (0 : 0.01 : N);

%-----plot    
figure
hold on
ylabel('y(x)')
xlabel('x')

TitleString = sprintf('ornament ease in/out functions');
title(TitleString);

xlim([0 1.7]);
ylim([0 1.7]);

LegendString = cell(1,7);
LIdx = 1;

y = n;
plot(n,y)
LegendString{LIdx} = sprintf('lineal'); LIdx=LIdx+1;

%---------------------------------------------------------
y = n.^(1/3); 
plot(n,y)
LegendString{LIdx} = sprintf('ease out: pow <1'); LIdx=LIdx+1;

y = 1 - (1 - n).^3;
plot(n,y)
LegendString{LIdx} = sprintf('ease out: pow >1'); LIdx=LIdx+1;

y = sin(n * pi / 2);
plot(n,y)
LegendString{LIdx} = sprintf('ease out: sin'); LIdx=LIdx+1;

y = sqrt(1 - (n - 1).^2);
plot(n,y)
LegendString{LIdx} = sprintf('ease out: circle'); LIdx=LIdx+1;

%------------------------
y = n.^(3);
plot(n,y)
LegendString{LIdx} = sprintf('ease in: pow >1'); LIdx=LIdx+1;

y = 1 - (1 - n).^(1/3);
plot(n,y)
LegendString{LIdx} = sprintf('ease in: pow <1'); LIdx=LIdx+1;

y = 1 - cos(n * pi / 2);
plot(n,y)
LegendString{LIdx} = sprintf('ease in: cos'); LIdx=LIdx+1;

y = 1 - sqrt(1 - n.^2);
plot(n,y)
LegendString{LIdx} = sprintf('ease in: circle');

legend(LegendString,'Location','best')
hold off

%===========================================================
%===========================================================
figure
subplot(2,1,1)
hold on

ylabel('y(x)')
xlabel('x')

TitleString = sprintf('ornament sigmoid-like functions');
title(TitleString);

xlim([0 1.7]);
ylim([0 1.0]);

LegendString = cell(1,2);
LIdx = 1;

y = n;
plot(n,y)
LegendString{LIdx} = sprintf('lineal'); LIdx=LIdx+1;

%-------------------------------------------------------------
y = 1-exp(-(n/0.55).^4);
plot(n,y)
LegendString{LIdx} = sprintf('ease in-out: Weibull SE'); LIdx=LIdx+1; %Weibull "stretched exponential"

for i = 1:numel(n)
    if n(i)<0.5
        y(i) = 4*n(i).^3;
    else
        y(i) = 1 - ((-2*n(i) + 2).^3) / 2;
    end
end
plot(n,y)
LegendString{LIdx} = sprintf('ease in-out: pow'); LIdx=LIdx+1;

y = -(cos(pi * n) - 1) / 2;
plot(n,y)
LegendString{LIdx} = sprintf('ease in-out: cos'); LIdx=LIdx+1;

for i = 1:numel(n)
    if (n(i) < 0.5)
        y(i) = (1 - sqrt(1 - (2 * n(i)).^2)) / 2;
    else
        y(i) = (sqrt(1 - (-2 * n(i) + 2).^2) + 1) / 2;
    end
end
plot(n,y)
LegendString{LIdx} = sprintf('ease in-out: circle');

legend(LegendString,'Location','best')
hold off

%----------------------------------------------------------
subplot(2,1,2)
hold on
ylabel('y(x)')
xlabel('x')

TitleString = sprintf('ornament inverse sigmoid-like functions');
title(TitleString);

xlim([0 1.7]);
ylim([0 1.0]);

LegendString = cell(1,2);
LIdx = 1;

y = n;
plot(n,y)
LegendString{LIdx} = sprintf('lineal'); LIdx=LIdx+1;

%----------------------------
y = 0.6*(-log(1-0.990241559588*n)).^(1/3);
plot(n,y)
LegendString{LIdx} = sprintf('ease out-in: inv Weibull SE'); LIdx=LIdx+1;

for i = 1:numel(n)
    if n(i)<0.5 
        y(i) = (0.25*n(i)).^(1/3);   
    else
        y(i) = -((2-2*n(i)).^(1/3)-2)/2;
    end
end
plot(n,y)
LegendString{LIdx} = sprintf('ease out-in: pow'); LIdx=LIdx+1;

y = acos(-2*n+1)/pi; 
plot(n,y)
LegendString{LIdx} = sprintf('ease out-in: arccos'); LIdx=LIdx+1;

for i = 1:numel(n)
    if (n(i) < 0.5)
	    y(i) = (sqrt(1 - (2*n(i) - 1).^2)) / 2;
    else
        y(i) = 1 - sqrt(n(i) - n(i).^2);
    end
end
plot(n,y)
LegendString{LIdx} = sprintf('ease out-in: circle'); LIdx=LIdx+1;

legend(LegendString,'Location','best')
hold off

