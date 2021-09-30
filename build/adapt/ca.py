import sys 
import pandas as pd
#def main(argv):
n = int(sys.argv[1])
df = pd.read_csv("./consistent.csv")
num = 0
ma = df.max()
me = df.mean()
for i in range(n):
    j = (-1) - i
    k = df.iloc[df.index[j]]
    #print(k)
    num = num + k
print("sum:",num)
print("me:",(ma-me)/me)
