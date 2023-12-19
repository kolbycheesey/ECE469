# Python3 implementation of Optimal page replacement Algorithm in Operating Systems. 
from collections import deque
from operator import itemgetter
import pandas as pd

def Optimal(pages, n, capacity):
    #To store the current frame
    #"N/A" represent empty frame
    frame = ["N/A"]*capacity 

    #To store the pages in in a queue
    pages_queue = deque()

    for page in pages:
        pages_queue.append(page)

    #To store the time
    time = 0

    #To store the output
    output = pd.DataFrame(index=range(0,capacity))

    # Start from initial page  
    page_faults = 0
    
    for i in range(n):
        # Check if the set can hold more pages  
        if (frame.count("N/A") > 0):
            # Insert it into set if not present  
            # already which represents page fault  
            if (pages[i] not in frame): 
                frame[frame.index("N/A")] = pages[i]

                # increment page fault  
                page_faults += 1

                #Put in the dataframe
                output[time] = frame

        # If the set is full then need to perform Optimal Algorithm  
        # i.e. replace page that will not be used for longest period of time in the future 
        else: 
              
            # Check if current page is not already present in frames 
            if (pages[i] not in frame): 
                
                time_to_use = [999]*capacity
                for j in range(0,len(frame)):
                    if frame[j] in pages_queue:
                        time_to_use[j] = pages_queue.index(frame[j])
                
                index = max(enumerate(time_to_use), key=itemgetter(1))[0]
                
                # Replace the indexes page
                frame[index] = pages[i]
    
                # Increment page faults  
                page_faults += 1

                #Put in the dataframe
                output[time] = frame
            else:
                temp = ["N/A"]*capacity
                temp[frame.index(pages[i])] = "hit"
                output[time] = temp
        time += 1
        pages_queue.popleft()

    print_graph(output, pages,capacity, page_faults)
    
# This is the function used to print out the simulated graph in a nice way
def print_graph(output, pages,capacity, page_faults):
    print("This is the simulated graph using Optimal page replacement algorithm:")
    
    for page in pages:
        print(str(page), end = '\t')
    print('\n')
    for c in range(0,capacity):
        for col in output:
            if(output[col][c] != "N/A"):
                print(output[col][c], end='\t')
            else:
                print(" ", end='\t')
        print('\n')

    print("\nNumber of page fault: " + str(page_faults))

# Driver code  
if __name__ == '__main__':

    print('\n')
    print("Input the reference string reference string.\n")
    #print("Please input the reference string separated by comma: (e.g. 7,0,1,2,0,3,0,4,2,3,0,3,0,3,2,1,2,0,1,7,0,1)")

    #pages_input = input()
    #pages = pages_input.split(',')
    #pages = [int(i) for i in pages] 
    pages = [1,4,5,6,6,7,3,5,4,4,3,7,9,8,7,4]
    print(pages)

    capacity = int(input("Please input the number of frame:\n"))
    
    n = len(pages)

    Optimal(pages, n, capacity)
    

