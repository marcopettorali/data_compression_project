import pandas as pd
import os
from stats_util import *
from matplotlib import pyplot as plt


def analyze():
    csv_file = max(["csv/" + f for f in os.listdir('csv')], key=os.path.getctime)

    print("Analyzing " + csv_file)
    df = pd.read_csv(csv_file, sep=';')

    ret = {}

    # collect records into ret dict
    for index, row in df.iterrows():
        if row['type'] not in ret:
            ret[row['type']] = {}
        if row['name'] not in ret[row['type']]:
            ret[row['type']][row['name']] = {}
        if row['dataset'] not in ret[row['type']][row['name']]:
            ret[row['type']][row['name']][row['dataset']] = []
        ret[row['type']][row['name']][row['dataset']].append(row['value'])

    # normalize compression_time per megabyte
    datasets_sizes = {}
    for name in ret["dataset_size"]:
        for dataset in ret["dataset_size"][name]:
            datasets_sizes[dataset] = ret["dataset_size"][name][dataset]

    for name in ret["compression_time"]:
        for dataset in ret["compression_time"][name]:
            ret["compression_time"][name][dataset] = [x / (datasets_sizes[dataset][0] / 1000) for x in ret["compression_time"][name][dataset]]

    # convert compressed size in compression ratio
    for name in ret["compressed_size"]:
        for dataset in ret["compressed_size"][name]:
            ret["compressed_size"][name][dataset] = [datasets_sizes[dataset][0] / x for x in ret["compressed_size"][name][dataset]]

    # compute 95th confidence intervals
    for type in ret:
        for name in ret[type]:
            for dataset in ret[type][name]:
                ret[type][name][dataset] = s_mean_confidence_interval(ret[type][name][dataset]) if len(ret[type][name][dataset]) > 1 else ret[type][name][dataset][0]

    for type in ret:

        if type == "dataset_size":
            continue
        elif type == "compression_time":
            title = "Compression Time"
            plt.ylabel("Time per Megabyte (ms/MB)")
        elif type == "random_access_time":
            title = "Random Access Time"
            plt.ylabel("Access time (us/B)")
        elif type == "sequential_access_time":
            title = "Sequential Access Time"
            plt.ylabel("Access time (us/B)")
        elif type == "compressed_size":
            title = "Compression Ratio (Original Size/Compressed Size)"

        yss = []
        yerrs = []
        labels = [x.split("/")[1] for x in list(ret[type][name])]
        names = list(ret[type])
        for name in ret[type]:
            if isinstance(list(ret[type][name].values())[0], list):
                yss.append([(x[0] + x[1]) / 2 for x in ret[type][name].values()])
                yerrs.append([(x[0] - x[1]) / 2 for x in ret[type][name].values()])
            else:
                yss.append(list(ret[type][name].values()))

        if len(yerrs) != 0:
            width = 1 / (len(yerrs) + 1)
            xss = list(range(len(labels)))

            for i, ys in enumerate(yss):
                xs = [x + i * width for x in xss]
                plt.bar(xs, ys, width, zorder=3, label=names[i])
                plt.errorbar(xs, ys, yerr=yerrs[i], fmt='none', capsize=5, ecolor='black', zorder=5)
                plt.xticks([x + len(ys)*width/2 for x in xss], labels)
        else:
            for i, ys in enumerate(yss):
                xs = [x + i * width for x in xss]
                plt.bar(xs, ys, width, zorder=3, label=names[i])
                plt.xticks([x + len(ys)*width/2 for x in xss], labels)

            

        plt.grid(axis='y', alpha=0.75)
        plt.legend()

        plt.title(title)
        plt.tight_layout()


        # save plot
        if not os.path.exists("out"):
            os.makedirs("out")
        plt.savefig("out/" + type + ".png")
        plt.show()


if __name__ == '__main__':
    analyze()