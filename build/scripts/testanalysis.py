# To change this template, choose Tools | Templates
# and open the template in the editor.

# This is mostly an example build by Dirk and will not be used for the dg paper!

import scipy.stats
import numby as np
import matplotlib
matplotlib.use("PDF")
import matplotlib.pyplot as plt
import sys
import os
import csv

###############################################################################
## Calculate the confidence Intervall for the given array
##
## @param data The array to calculate the confidence-interval about
## @param confidence The confidence to calculate
##        (the probability that an Element is in the interval)
##
## @return The lower and upper bound of the confidence interval
###############################################################################
def mean_confidence_interval(data, confidence=0.95):
    a = 1.0*np.array(data)
    n = len(a)
    m, se = np.mean(a), scipy.stats.stderr(a)
    # calls the inverse CDF of the Student's t distribution
    h = se * scipy.stats.t._ppf((1+confidence)/2., n-1)
    return m-h, m+h

###############################################################################
## Calculate the mean value for the given array
##
## @param data The array to calculate the mean about
##
## @return The mean value
###############################################################################
def mean(data):
    a = 1.0 * np.array(data)
    return np.mean(a)

###############################################################################
## Read Data from the given File. The file has to exist in csv notation with
## comma (,) as delimiter.
##
## @param data The array to calculate the mean about
##
## @return The mean value
###############################################################################
def read_data(filename, parameter_names = ["config"]):
    # Identify the factors in the list
    def gather_factors( head_data, parameter_keys):
        factors_ = []
        for i in xrange(len(head_data)):
            c = head_data[i]
            for p in parameter_keys:
                if c == p:
                    break
            else:
                factors_.append((c,i))
        return factors_

    # Identify the parameters in the list
    def gather_parameters(head_data, parameter_keys):
        parameters_ = []
        for i in xrange(len(head_data)):
            c = head_data[i]
            for p in parameter_keys:
                if c == p:
                    parameters_.append((c, i))
                    break
        return parameters_

    f = open(filename)
    r = csv.reader(f)
    data = [[c for c in row] for row in r]

    #factors = gather_factors(data[0], ["config"])
    #parameters = gather_parameters(data[0], ["config"])
    factors = gather_factors(data[0], parameter_names)
    parameters = gather_parameters(data[0], parameter_names)
    
    # remove the headers
    data = data[1:]
    data.sort()

    return Data(data, factors, parameters)

###############################################################################
## This class holds the Data and offers method to access them more convinient
###############################################################################
class Data():

    ###########################################################################
    ## Constructor
    ##
    ## @param data A 2-dimensional array holding the data
    ## @param factors a List denoting the columns holding factos
    ## @param parameters a List denoting the the columns holding parameters
    ###########################################################################
    def __init__(self, data, factors, parameters):
        self.data = data
        self.factors = factors
        self.parameters = parameters

    ###########################################################################
    ## return the whole data as 2 dimensional map
    ##
    ## @return the whole data as 2 dimensional map
    ###########################################################################
    def all(self):
        return self.data

    ###########################################################################
    ## get all rows holding in the column referred by parameter_name one of
    ## the given configs.
    ##
    ## @param configs The positive matching values to be in the comlumn
    ## @param parameter_name The name of the column to be filtered
    ##
    ## @return all rows holding the requested values in the given column
    ###########################################################################
    def filter_config(self, configs, parameter_name="config"):
        new_data = []
        for config in configs:
            i = data.index_of_parameter(parameter_name)
            for row in self.data:
                if row[i] == config:
                    new_data.append(row)
        return Data(new_data, self.factors, self.parameters)

    ###########################################################################
    ## find the column denoting the given parameter
    ##
    ## @param parameter_name The name of the parameter to find
    ##
    ## @return The column holding the data of the parameter
    ###########################################################################
    def index_of_parameter(self, parameter_name):
        for parameter in self.parameters:
            if parameter[0] == parameter_name:
                return parameter[1]
        raise ValueError("Illegal parameter %s: %s" % (parameter_name, ",".join((p[0] for p in self.parameters))))

    ###########################################################################
    ## find the column denoting the given factor
    ##
    ## @param factor_name The name of the factor to find
    ##
    ## @return The column holding the data of the factor
    ###########################################################################
    def index_of_factor(self, factor_name):
        for factor in self.factors:
            if factor[0] == factor_name:
                return factor[1]
        raise ValueError("Illegal factor %s: %s" % (factor_name, ",".join((f[0] for f in self.factors))))

    ###########################################################################
    ## get all Values of a given factor
    ##
    ## @param factor_name The name of the factor to get values for
    ##
    ## @return The column denoted by the given factor
    ###########################################################################
    def get_values(self, factor_name):
        # Convert Value to double. Return 0 if there is no valid double
        def convert_value(v):
            try:
                return float(v)
            except ValueError:
                return 0.0
        i = self.index_of_factor(factor_name)
        values = [row[i] for row in self.data]

        return np.array([convert_value(v) for v in values])

def get_storage_label(data, mode = None):
    storage_label_mapping = {"16kchunks": "16KB chunks", "2ssd": "2 SSD", "4ssd": "4 SSD","1ssd": "1 SSD", "1worker": "1 client", "2worker": "2 clients",
        "256KB": "256 KB", "512KB": "512 KB", "64KB": "64 KB", "mem": "RAM", "noaux2": "minimal auxiliary index", "noaux": "reduced auxiliary index"}
    config_index = data.index_of_parameter("config")
    labels = []
    for row in data.all():
        factor_id = row[config_index]
        if mode == "worker" and factor_id == "4ssd":
            label = "4 clients"
        elif mode == "bi_size" and factor_id == "4ssd":
            label = "128 KB"
        elif mode == "chunks" and factor_id == "4ssd":
            label = "8KB chunks"
        elif mode == "noaux" and factor_id == "4ssd":
            label = "full auxiliary index"
        else:
            label = storage_label_mapping[factor_id]
        labels.append(label)
    return labels

def tp_chart(data, new_xticks, filename):
    l = len(data.all())
    width = 0.5
    ind = np.arange(l)

    plt.figure(figsize=figure_size)
    run1_means = data.get_values("run1-tp-Mean")
    run1_err = None
    run1_err = data.get_values("run1-tp-Interval")
    run2_means = data.get_values("run2-tp-Mean")
    run2_err = None
    run2_err = data.get_values("run2-tp-Interval")
#print run1_means, run2_means

    p1 = plt.bar([i * 3 for i in ind],run1_means, yerr=run1_err, ecolor="k", color="0.5", label="Generation 1")

    p2 = plt.bar([(i * 3) + 1 for i in ind],run2_means, yerr=run2_err, ecolor="k", color="w", label="Generation 2")

    plt.ylabel("Throughput MB/s")
    xticks = ["" for i in xrange(l * 3)]
    for i in xrange(l):
        xticks[(i * 3) + 1 ] = new_xticks[i]
    ind = np.arange(l * 3)
    plt.xticks(ind, xticks)
    leg = plt.legend(loc=0)

    plt.savefig(os.path.join(output_dir, filename))

profile_columns = [
        "profile-chunking",
        "profile-ci",
        "profile-bi",
        "profile-storage",
        "profile-log",
        "profile-lock"]

def get_profile_values(data, run, data_kind = "Mean", normalized=False):
    values = []
    sum = np.empty([len(data.all())])
    for i in xrange(len(data.all())):
        sum[i] = 0
    for profile_column in profile_columns:
        value = data.get_values("%s-%s-%s" % (run, profile_column, data_kind))
        if profile_column == "profile-total":
            value = value - sum
        sum = sum + value
        values.append(value)
    if normalized:
        return [value / sum for value in values]
    else:
        return [value / 1000 for value in values]

def profile_chart(data, new_xticks, filename, normalized=False, bcf=False, bloom=False):
    profile_names = ["Chunking","Chunk Index",  "Block Index","Storage","Log", "Lock", "BCF", "Bloom"]
    profile_colors = ["1.0",     "0.9"     ,    "0.7"        ,"0.5"         ,"0.3","0.1", "0.4", "0.2"]
    def add_run_profile_bar(means, index_offset):
        top = np.empty([len(means[0])])
        for i in xrange(len(means[0])):
            top[i] = 0
        for profile_index in xrange(len(means)):
            profile_data = means[profile_index]
            profile_color = profile_colors[profile_index]
            profile_name = None
            if index_offset == 0:
                profile_name = profile_names[profile_index]

            if not (bcf == False and profile_index == 6) and not (bloom == False and profile_index == 7):
                plt.bar([(i * 3) + index_offset for i in ind],profile_data,bottom=top,color=profile_color,label=profile_name)
                top = top + profile_data

    l = len(data.all())
    width = 0.5
    ind = np.arange(l)

    plt.figure(figsize=figure_size)
    run1_means = get_profile_values(data, "run1", "Mean", normalized)
    run2_means = get_profile_values(data, "run2", "Mean", normalized)

    add_run_profile_bar(run1_means,0);
    add_run_profile_bar(run2_means,1);

    if(normalized):
        plt.ylabel("Ratio on total running time")
        locs, labels = plt.yticks()
        labels = ["%s%%" % (i * 100) for i in locs]
        plt.yticks(locs, labels)
    else:
        plt.ylabel("Thread wall clock time")
        locs, labels = plt.yticks()
        labels = ["" for i in locs]
        plt.yticks(locs, labels)

    xticks = ["" for i in xrange(l * 3)]
    for i in xrange(l):
        xticks[(i * 3) + 1 ] = new_xticks[i]
    ind = np.arange(l * 3)
    plt.xticks(ind, xticks)
    (xmin,xmax) = plt.xlim()
    plt.xlim(xmax=xmax*1.6)
    leg = plt.legend(loc=0)
    f = leg.get_frame()
    f.set_alpha(0.0)
    plt.savefig(os.path.join(output_dir,filename))

def summary_profile_chart(data, configs, name, mode = None):
    data = data.filter_config(configs)
    storage_labels = get_storage_label(data, False)

    filename = "profile-%s-chart.pdf" % name
    profile_chart(data, storage_labels, filename)

def summary_chart(data, configs, name, mode = None):
    data = data.filter_config(configs)
    storage_labels = get_storage_label(data, mode=mode)

    filename = "tp-%s-chart.pdf" % name
    tp_chart(data, storage_labels, filename)


if __name__ == "__main__":
    print "Hello World"
#    data_file = sys.argv[1]
#    data = read_data(data_file)
#    summary_chart(data, ["1ssd", "2ssd", "4ssd", "mem"], "devices")
#    summary_chart(data, ["1worker", "2worker", "4ssd"], "worker", mode="worker")
#    summary_chart(data, ["4ssd","16kchunks"], "chunks", mode="chunks")
#    summary_chart(data, ["noaux2", "4ssd"], "noaux", mode="noaux")
#    summary_profile_chart(data, ["1ssd", "2ssd", "4ssd", "mem"], "devices")
