function [ iterative_serial, recursive_serial ] = average_calculator( )
%AVERAGE_CALCULATOR Summary of this function goes here
%   Detailed explanation goes here
    
    iterative_serial = zeros(1,5);
    recursive_serial = zeros(1,5);
    counter = 1;
    A = textread('results-serial');
    for data=16:20
        it_sum = 0;
        rec_sum = 0;
        for i=1:size(A,1)
            if(A(i,1)==data)
                it_sum = it_sum + A(i,2);
                rec_sum = rec_sum + A(i,3);
            end    
        end
    
        iterative_serial(1,counter) = it_sum/20;
        recursive_serial(1, counter) = rec_sum/20;
        counter = counter+1;
    end
end

