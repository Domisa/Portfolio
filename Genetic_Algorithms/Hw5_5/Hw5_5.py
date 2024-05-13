import random
import math

def chromosomeCreate(chromosomeLength, population):

    #The chromosomes array contains the binary encoded chromosomes as lists.
    chromosomes = []
    for i in range(population): 
        binaryEncoded = []
        for j in range(chromosomeLength):
            strand = random.randint(0,1)
            binaryEncoded.append(strand)
        
        chromosomes.append(binaryEncoded)
        

    return chromosomes

def chromosomeSplit(chromosomes, chromosomeLength):

    splitSpot = chromosomeLength//2
    Xofx = []
    Yofy = []
    for i in chromosomes:
        X = []
        Y = []
        for j in range(splitSpot):
            X.append(i[j])
        for j in range(splitSpot,chromosomeLength):
            Y.append(i[j])
    
        Xofx.append(X)
        Yofy.append(Y)

    # splitSpot is the size of each array
    return Xofx, Yofy, splitSpot

def conversionBaseTen(chromosomeLength, chromosomeGeneration):
    currentGenIntegers = []
    # print(chromosomeLength)
    # print(chromosomeGeneration)
    for i in chromosomeGeneration:
        baseTen = 0
        
        for j in range(len(i)):
            exponent = len(range(chromosomeLength-1)) - j
            #print(exponent)
            baseTen += i[j]*2**exponent
        currentGenIntegers.append(baseTen)
    
    return currentGenIntegers
        
def conversion8Bit(formerlyBinaryArrays, populationSize):
    nowWithinRange = []
    decimator = populationSize/(256-1)
    for i in formerlyBinaryArrays:
        newValue = i*decimator-3
        nowWithinRange.append(newValue)
    
    return nowWithinRange


def evaluate(listX, listY):
    listValues = []
    
    for x, y in zip(listX, listY):
        Equation = (1-x)**2*math.exp(-x-(y+1)**2) - (x-x**3-y**3)*math.exp(-x**2-y**2)
        listValues.append(Equation)
    
    #print(listValues)

    return listValues


def rouletteWheel(evaluatedList):

    summed = sum(evaluatedList)

    #Intended to be the weighted list for choices method
    weightList = []

    for i in range(len(evaluatedList)):

        #if the weights are not inverted, then the highest values would have the highest chance of being changed
        weight= 1-(evaluatedList[i]/summed)
        weightList.append(weight)

    #This selects the chromosomes to breed by their results, it
    spot1 = random.choices(evaluatedList,weights=weightList, k=1)
    spot2 = random.choices(evaluatedList,weights=weightList, cum_weights=None,k=1)

    #This returns the index location of the spots
    spot1 = evaluatedList.index(spot1[0])
    spot2 = evaluatedList.index(spot2[0])

    while spot1 == spot2: #or evaluatedList.count(spot1) != 1 or evaluatedList.count(spot2)!=1:
        spot2 = random.choices(evaluatedList,weights=weightList)
        spot2 = evaluatedList.index(spot2[0])
    
    
    return spot1, spot2

def crossover(evaluationList, listX, listY, chromosomeLength, crossProb):

    #Crossover should happen for every generation, but how much crossover do we need?
    #it doesn't make much sense for a crossover to occur with more than half of the array size, does it?
    #Just in case, There will be two selectAmount variables.

    doesItHappen = random.uniform(0, 1)

    ###########################################################
    # swapHowMuch = random.randint(1, chromosomeLength//2)
    ###########################################################
    if doesItHappen < crossProb:

        selectAmount = random.randint(1, chromosomeLength//2)

        ###############################################################################################################################
        #If a value in the list is greater than the average, then it will be cloned, not bred.

        #average = sum(evaluationList)/len(evaluationList)
        # tempListX = []
        # tempListY = []
        # newX = []
        # newY = []

        
        # for each evaluated item in the list, check if the average is greater
        # if not, then that item is added to the blender
        # tempList is for the blender, newX and newY are cloned for the next Gen

        #This was the previous method, leveraging the average to toss out every bad value.

        # for i in range(len(evaluationList)):
        #     if evaluationList[i] <= average:
                
        #         tempListX.append(listX[i])
        #         tempListY.append(listY[i])
            
        #     else:
                
        #         newX.append(listX[i])
        #         newY.append(listY[i])
                
              
        
        # print(len(tempListX))

        #This iterates over population size using already accessed data
        #########################################################################################################################

        for i in range(len(evaluationList)):
            
            #previous method
            ######################################################################################
            # spot1 = random.randint(0,chromosomeLength//2-1)
            # spot2 = random.randint(0,chromosomeLength//2-1)

            # while spot1 == spot2 or spot1 > len(tempListX)-1 or spot2 > len(tempListY)-1:
            #     spot1 = random.randint(0,chromosomeLength//2-1)
            #     spot2 = random.randint(0,chromosomeLength//2-1)
            ###########################################################################################


            #currentmethod
            ###########################################################################################
            spot1, spot2 = rouletteWheel(evaluationList)

            #Switcheroo
            tempValue = listX[spot1][selectAmount:]
            listX[spot1][selectAmount:] = listX[spot2][selectAmount:]
            listX[spot2][selectAmount:] = tempValue

            tempValue = listY[spot1][selectAmount:]
            listY[spot1][selectAmount:] = listY[spot2][selectAmount:]
            listY[spot2][selectAmount:] = tempValue
            #########################################################################################################################

        # From previous method
        #######################################################################################################################
            # print(spot1, spot2)
            # tempValue = tempListX[spot1][selectAmount:]

            # # print(len(tempListX))
            # # print(tempValue)

            # tempListX[spot1][selectAmount:] = tempListX[spot2][selectAmount:]
            # tempListX[spot2][selectAmount:] = tempValue
        
            # tempValue = tempListY[spot1][selectAmount:]
            # tempListY[spot1][selectAmount:] = tempListY[spot2][selectAmount:]
            # tempListY[spot2][selectAmount:] = tempValue
        # newY = newY + tempListY
        # newX = newX + tempListX
        ##################################################################################################################3

            

    else:

        # print("it didn't happen")
        return listX, listY
    
    #previous return statement
    #return newX, newY

    return listX, listY



    
def mutate(mutateProbs, populationX, populationY, chromosomeLength):
    #Does it happen?
    doesIt = random.uniform(0.0, 1)
    #Well if it does happen...
    if doesIt < mutateProbs:
        
        whichCoordPoint = random.uniform(0.0, 1)

        if whichCoordPoint > .5:
        #This randomly finds where the mutation will be
            whichOne = random.randint(0, len(populationX)-1)
            whichBit = random.randint(0, chromosomeLength-1)

            #If the item is 1, switch it to 0, and vice versa.
            if int(populationX[whichOne][whichBit]) == 1:
                populationX[whichOne][whichBit] = 0

            else:
                populationX[whichOne][whichBit] = 1
        
        else:

            whichOne = random.randint(0, len(populationX)-1)
            whichBit = random.randint(0, chromosomeLength-1)

            #If the item is 1, switch it to 0, and vice versa.
            if int(populationY[whichOne][whichBit]) == 1:
                populationY[whichOne][whichBit] = 0

            else:
                populationY[whichOne][whichBit] = 1
    
    return populationX, populationY

def main():
    fullchromosomeLength = 16
    population = 8
    generations = 1000
    crossoverProbability = .7
    mutationProbability = .001

    # fullchromosomeLength = int(input("Enter Chromosome length"))
    # population = int(input('Enter Population size'))
    # generations = int(input('Enter number of generations'))
    # crossoverProbability = int(input('Enter crossover Probability'))
    # mutationProbability = int(input('Input mutation probability'))

    currentGen = chromosomeCreate(fullchromosomeLength, population)
    currentGenX, currentGenY, chromosomeLength= chromosomeSplit(currentGen, fullchromosomeLength)

    for i in range(generations):

        #print(currentGenX)

        convertedGenX = conversionBaseTen(chromosomeLength, currentGenX)
        convertedGenY = conversionBaseTen(chromosomeLength, currentGenY)

        # print(convertedGenX)
        # print(convertedGenY)

        convertedGenX = conversion8Bit(convertedGenX, population)
        convertedGenY = conversion8Bit(convertedGenY, population)

        # print(convertedGenX)
        # print(convertedGenY)

        currentGenResults = evaluate(convertedGenX, convertedGenY)

        currentGenX, currentGenY = mutate(mutationProbability, currentGenX, currentGenY, chromosomeLength)

        #pick the preferred print format for your convenience.
        #print(currentGenResults)
        print(sorted(currentGenResults, reverse=True))


        # print(currentGenX)
        # print(currentGenY)

        currentGenX, currentGenY = crossover(currentGenResults, currentGenX, currentGenY, chromosomeLength, crossoverProbability)

        # print(currentGenX)
        # print(currentGenY)
main()