## LastClose
inData = []
with open("TestData2.csv","r") as f:
    for line in f:
        # timestamp, open, high, low, close, volume
        splitLine = line.split(",")
        # skip information lines
        if len(splitLine) == 1:
            continue
        # skip the header line
        if splitLine[1] == "open":
            continue
        time = splitLine[0]
        op = float(splitLine[1])
        hi = float(splitLine[2])
        lo = float(splitLine[3])
        cl = float(splitLine[4])
        vol= int(splitLine[5])
        inData.append( [time, op, hi, lo, cl, vol] )

# queue to keep track of open positions
UPnL = 0
PnL  = 0
maxGain = 0
maxDrawdown = 1000000.0
positions = []
shortShares = False
# inData is in reverse chronological order
for i in range( len(inData)-1, -1, -1 ):
    # skip the first candle
    if i == len(inData)-1:
        continue
    # LastClose strategy
    if inData[i][4] > inData[i+1][4]:
        # closing short positions:
        if shortShares:
            outPrice = positions.pop(0)
            PnL += outPrice - inData[i][4]
            if len(positions) == 0:
                shortShares = False
        # buying long position
        else:
            positions.append(inData[i][4])
    #elif inData[i][4] < inData[i+1][4]:
    else:
        # selling a long position
        if (len(positions) > 0) and (not shortShares):
            outPrice = positions.pop(0)
            PnL += (inData[i][4] - outPrice)
        # opening a short position
        else:
            shortShares = True
            positions.append(inData[i][4])
    # Mark To Market: (Current Price - Avg Price) * Pos Size
    if len(positions) > 0:
        UPnL = (inData[i][4] - sum(positions)/len(positions)) * len(positions)
    maxGain = PnL+UPnL if PnL+UPnL > maxGain else maxGain
    maxDrawdown = PnL+UPnL if PnL+UPnL < maxDrawdown else maxDrawdown
    print(inData[i][4])
    print(UPnL)
print("Maximum gain was: $"+str(maxGain))
print("Maximum drawdown was: $"+str(maxDrawdown))
print("Ending position size is "+str(len(positions)))
