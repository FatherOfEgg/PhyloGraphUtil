# PhyloGraphUtil

A tool to compare and convert between different network formats.
## Features

- Compare between 2 networks using
  - Robinson Foulds
  - Jaccard index
  - F1 Score
- Convert one format into another
  - **Note:** Most information about the network for certain things/attributes will be lost, such as internal node names, length, weights, etc.
  - While the graph may be the same in terms of structure, the output may look different, as in, `GML -> Extended Newick -> GML` may or may not produce something that looks different.

## Formats

- ✔ - Supported
- ❌ - Not supported
- 📎 - Planned

| Formats              | Extension(s)  | Open | Save |
|----------------------|---------------|------|------|
| GML                  |    `.gml`     | ✔    | ✔    |
| Extended Newick      | `.enwk, .ewk` | ✔    | ✔    |
| Admixtools admixture |   `.admix`    | ✔    | ✔    |
| Simple admixture     |   `.sadmix`   | 📎   | 📎    |
| Treemix              |  `.treemix`   | ✔    | ❌   |

## Usage/Examples

```
PhyloGraphUtil

USAGE:
        PhyloGraphUtil <SUBCOMMAND>

FLAGS:
        -h      Prints help information for this and each subcommand.

SUBCOMMANDS:
        convert Converts from one graph format to another.
        compare Compares one graph to another using a specified method.
        print   Prints basic info of the read-in graph(s).

FORMATS:
        GML     .gml
        ENWK    .enwk, .ewk
        ADMIX   .admix
        TREEMIX .treemix
```

Converting GML to Extended Newick:
```
PhyloGraphUtil convert test.gml ENWK
```

Comparing 2 different networks using Robinson Foulds:
```
PhyloGraphUtil compare rf A.gml B.enwk
```

## Build

Simply run `make`, and the executable will be found in the `bin` folder.

## Roadmap

- More formats
- Different methods for conversion
- Better way to open and parse the different formats
  - Maybe supporting different file extensions for the same format, for example, `enwk` or `ewk` for Extended Newick
- When converting to certain formats, be able to customize the output, such as including internal names and whatnot

## Credit

- [Jonker-Volgenant's algorithm for Linear Assignment Problem](https://github.com/yongyanghz/LAPJV-algorithm-c)
