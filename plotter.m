clear all
clc

x = [16, 17, 18, 19, 20];
[iterative_serial recursive_serial] = average_calculator();

A = textread('results-tabular.txt');
B = textread('data');
%sort rows of B according to 1st column, which corresponds to number of
%processes
B = sortrows(B, 1);
%remove rows, which correspond to 1 process because time is almost 0
B = B(13:end, 1:end);
 
pows = [2, 4, 8, 16, 32, 64, 128];    
for data= 16:20
    cases = 1;
    time = zeros(2,7);
    cnt = 1;
    for i=1:size(A,1)
        if(A(i,2)==data)
          time(1, cnt) = A(i,4);
          cnt = cnt+1;
        end
    end
    
    time_grid = zeros(2,7);
    cnt1 = 1;
    K = B(B(:,2)==data,:);  
    for j=1:7
       L = K(K(:,1)==pows(j),:);
       mean_value = mean(L);
       time_grid(1, cnt1) = mean_value(4);
       cnt1 = cnt1+1;
    end
 
    figure()
    y = [time; time_grid];
    h = bar(y);
    hold on
    colormap(summer(length(time)));
    grid on
    it_time = iterative_serial(cases);
    rec_time = recursive_serial(cases);
    plot(xlim, [it_time it_time],...
             'LineWidth',2,...
             'Color',[.8 .8 .8]);
    plot(xlim, [rec_time rec_time],...
             'LineWidth',2,...
             'Color',[.3 .3 .3]);  
    legend('iterative serial', 'recursive_serial');
    title(sprintf('Bitonic sort with dataset of size %d ', 2^data));
    xlabel('Mpi processes');
    ylabel('Time (s)');
    l = cell(1,7);
    l{1}='2'; l{2}='4'; l{3}='8'; l{4}='16'; l{5}='32'; l{6}='64'; l{7}='128';    
    legend('2 processes', ...
            '4 processes', ...
            '8 processes', ...
            '16 processes', ...
            '32 processes', ...
            '64 processes', ...
            '128 processes', ...
            'iterative serial', ...
            'recursive serial');
   
    cases = cases+1;
end    