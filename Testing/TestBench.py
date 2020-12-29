## Input
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

## LastClose
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
print("LastClose:")
print("Maximum gain was: $"+str(maxGain))
print("Maximum drawdown was: $"+str(maxDrawdown))
print("Ending Marked To Market Change was: $"+str(PnL+UPnL))
print("Ending position size is "+str(len(positions)))

## SMACrossover
UPnL = 0
PnL  = 0
maxGain = 0
maxDrawdown = 1000000.0
positions = []
shortShares = False
j = 0
for i in range( len(inData)-1, -1, -1 ):
    j += 1
    # wait until all SMA buffers are full
    if i > len(inData)-21:
        continue
    # buy signal: 5SMA crosses over 20SMA
    Buffer5  = sum([x[4] for x in inData[i:i+5]])/5
    Buffer20 = sum([x[4] for x in inData[i:i+20]])/20
    if Buffer5 > Buffer20:
        # closing short positions:
        if shortShares:
            outPrice = positions.pop(0)
            PnL += outPrice - inData[i][4]
            if len(positions) == 0:
                shortShares = False
        # buying long position
        else:
            positions.append(inData[i][4])
    # sell signal
    if Buffer5 < Buffer20:
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
print("SMACrossover:")
print("Maximum gain was: $"+str(maxGain))
print("Maximum drawdown was: $"+str(maxDrawdown))
print("Ending Marked To Market Change was: $"+str(PnL+UPnL))
print("Ending position size is "+str(len(positions)))

## HPSMA
UPnL = 0
PnL  = 0
maxGain = 0
maxDrawdown = 1000000.0
positions = []
shortShares = False
j = 0
for i in range( len(inData)-1, -1, -1 ):
    j += 1
    # wait until all SMA buffers are full
    if i > len(inData)-21:
        continue
    # buy signal: 5SMA crosses over 20SMA
    Buffer5  = sum([x[4] for x in inData[i:i+5]])/5
    Buffer20 = sum([x[4] for x in inData[i:i+20]])/20

    if Buffer5 > Buffer20:
        # initial condition, must open a position of size 1 first
        if len(positions) == 0:
            positions.append(inData[i][4])
            shortShares = False
            continue
        # make sure we have a long position, and if we don't, flip it
        if shortShares:
            outPrice = positions.pop(0)
            PnL += (outPrice - inData[i][4])
            positions.append(inData[i][4])
            shortShares = False
    if Buffer5 < Buffer20:
        # initial condition, must open a position of size 1 first
        if len(positions) == 0:
            positions.append(inData[i][4])
            shortShares = True
            continue
        # make sure we have a short position, and if we don't, flip it
        if not shortShares:
            outPrice = positions.pop(0)
            PnL += inData[i][4] - outPrice
            positions.append(inData[i][4])
            shortShares = True
    # Mark To Market: (Current Price - Avg Price) * Pos Size
    if len(positions) > 0:
        UPnL = (inData[i][4] - sum(positions)/len(positions)) * len(positions)
    maxGain = PnL+UPnL if PnL+UPnL > maxGain else maxGain
    maxDrawdown = PnL+UPnL if PnL+UPnL < maxDrawdown else maxDrawdown
print("HPSMA:")
print("Maximum gain was: $"+str(maxGain))
print("Maximum drawdown was: $"+str(maxDrawdown))
print("Ending Marked To Market Change was: $"+str(PnL+UPnL))
print("Ending position size is "+str(len(positions)))
