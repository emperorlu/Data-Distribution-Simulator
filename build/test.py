#!/usr/bin/env python
# coding=utf-8
import pandas as pd 
df = pd.read_csv("./consistent.csv")
me = df.mean()
ma = df.max()
print(df.std())
print(me)
print(ma)
print((ma-me)/me)

