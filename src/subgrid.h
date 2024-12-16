#ifndef SUBGRIDLIST_H
#define SUBGRIDLIST_H

/*
 * Description: This is a header file for the LHAGrid class. It contains the
 *              class definition and the function definitions for the class.
 *              A LHAGrid object is created from a PDF grid file. Each object
 *              contains their own header at the start of the file, and the
 *              x, q, flavor IDs, and PDF values for each subgrid seperately.
 * 
 *              mcgen.cc will call for specific operations to be performed
 *              on the input grids. These operations are performed immediately
 *              when the output LHAGrid object is created. The output LHAGrid
 *              object is then written to a file which is defined inside mcgen.cc.
 *
 * Author: Lucas Kotz, Pavel Nadolsky
 * Date: July 18, 2023
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <string>
#include <map>
#include <iomanip>

std::ostream &precisionScientific(std::ostream &os)
{
  os << std::setw(15) << std::uppercase << std::scientific << std::setprecision(6);
  return os;
}

std::ostream &pdfPrecision(std::ostream &os)
{
  os << std::uppercase << std::scientific << std::setprecision(8);
  return os;
}

std::ostream &pdfSpacing(std::ostream &os)
{
  os << std::setw(16);
  return os;
}

class LHAGrid
{
private:
  std::vector<double> xValues;
  std::vector<double> qValues;
  std::vector<int> flavors;
  std::vector<double> pdfValues;

  std::vector<std::string> headers;
  std::vector<std::vector<double>> xValuesList;
  std::vector<std::vector<double>> qValuesList;
  std::vector<std::vector<int>> flavorsList;
  std::vector<std::vector<double>> pdfValuesList;

  int Ngrids = 0;

  std::string operation;
  std::vector<std::string> files;
  std::vector<double> weights;

  const double small=1.0e-10;

public:
  // contructor
  LHAGrid(std::string filename)
  {
    this->ReadLHAGrid(filename);
  }

  LHAGrid(std::string op, std::vector<std::string> inputfiles, std::vector<double> w = std::vector<double>()) : operation(op), files(inputfiles) 
  {
    if (w.empty()) 
      w = std::vector<double>(inputfiles.size(), 1.0); // Fill with ones if w is not provided


    if (op == "add")
    {
      // create a vector of LHAGrids from the input files read
      std::vector<LHAGrid *> LHAgrids;
      int Nfiles = inputfiles.size();
      for (int i = 0; i < Nfiles; i++)
      {
        LHAgrids.push_back(new LHAGrid(inputfiles[i]));
      }
      this->CompareLHAGrids(LHAgrids);  // check if the grids are compatible
      // If input grids have the same number of grids, then assign the output 
      // grid the appropirate number of subgrids.
      LHAGrid *A1 = LHAgrids[0];
      int Ngrids1 = A1->Ngrids;
      Ngrids = Ngrids1;

      // allocate size to the 2-dimensional PDF values vector
      pdfValuesList.resize(Ngrids);
      for (int i = 0; i < Ngrids; i++)
      {
        int ival1 = A1->pdfValuesList[i].size();
        int ival = ival1;
        pdfValuesList[i].resize(ival);
      }

      // Perform the addition of the PDF values weighted by their appropriate weights
      for (int isub = 0; isub < Ngrids; isub++)
        for (int i = 0; i < pdfValuesList[isub].size(); i++)
          for (int ifile = 0; ifile < Nfiles; ifile++)
            pdfValuesList[isub][i] += w[ifile] * LHAgrids[ifile]->pdfValuesList[isub][i];

      // copy the headers, xValues, qValues, flavor IDs from the first grid to the output grid
      std::vector<std::string> headers1 = A1->headers;
      headers = headers1;

      xValuesList.resize(Ngrids);
      std::vector<std::vector<double>> xValuesList1 = A1->xValuesList;
      for (int isub = 0; isub < Ngrids; isub++)
        for (int i = 0; i < xValuesList[isub].size(); i++)
        {
          xValuesList[isub].resize(xValuesList1[isub].size());
        } // for (int isub = 0; isub < Ngrids; isub++)
      xValuesList = xValuesList1;

      qValuesList.resize(Ngrids);
      std::vector<std::vector<double>> qValuesList1 = A1->qValuesList;
      for (int isub = 0; isub < Ngrids; isub++)
        for (int i = 0; i < qValuesList[isub].size(); i++)
        {
          qValuesList[isub].resize(qValuesList1[isub].size());
        } // for (int isub = 0; isub < Ngrids; isub++)
      qValuesList = qValuesList1;

      flavorsList.resize(Ngrids);
      std::vector<std::vector<int>> flavorsList1 = A1->flavorsList;
      for (int isub = 0; isub < Ngrids; isub++)
        for (int i = 0; i < flavorsList[isub].size(); i++)
        {
          flavorsList[isub].resize(flavorsList1[isub].size());
        } // for (int isub = 0; isub < Ngrids; isub++)
      flavorsList = flavorsList1;
    } // if (op == "add")

    if (op == "multiply")
    {
      // create a vector of LHAGrids from the input files read
      std::vector<LHAGrid *> LHAgrids;
      int Nfiles = inputfiles.size();
      for (int i = 0; i < Nfiles; i++)
      {
        LHAgrids.push_back(new LHAGrid(inputfiles[i]));
      }
      this->CompareLHAGrids(LHAgrids); // check if the grids are compatible
      // If input grids have the same number of grids, then assign the output 
      // grid the appropirate number of subgrids.
      LHAGrid *A1 = LHAgrids[0];
      int Ngrids1 = A1->Ngrids;
      Ngrids = Ngrids1;

      // allocate size to the 2-dimensional PDF values vector
      pdfValuesList.resize(Ngrids);
      for (int i = 0; i < Ngrids; i++)
      {
        int ival1 = A1->pdfValuesList[i].size();
        int ival = ival1;
        pdfValuesList[i].assign(ival, 1); // fills entire 2-dimension vector with 1
      }

      // Perform the multiplication of the PDF values weighted by their appropriate weights
      for (int isub = 0; isub < Ngrids; isub++)
        for (int i = 0; i < pdfValuesList[isub].size(); i++)
          for (int ifile = 0; ifile < Nfiles; ifile++)
          {
            double pdfValuetmp = LHAgrids[ifile]->pdfValuesList[isub][i];
            if (fabs(pdfValuetmp) < small) // checks if the magnitude of pdfValuetmp is too small
              pdfValuetmp = copysign(small, pdfValuetmp); // if so, then set it to small
            pdfValuesList[isub][i] *= pow(LHAgrids[ifile]->pdfValuesList[isub][i], w[ifile]);
          }

      // copy the headers, xValues, qValues, flavor IDs from the first grid to the output grid
      std::vector<std::string> headers1 = A1->headers;
      headers = headers1;

      xValuesList.resize(Ngrids);
      std::vector<std::vector<double>> xValuesList1 = A1->xValuesList;
      for (int isub = 0; isub < Ngrids; isub++)
        for (int i = 0; i < xValuesList[isub].size(); i++)
        {
          xValuesList[isub].resize(xValuesList1[isub].size());
        } // for (int isub = 0; isub < Ngrids; isub++)
      xValuesList = xValuesList1;

      qValuesList.resize(Ngrids);
      std::vector<std::vector<double>> qValuesList1 = A1->qValuesList;
      for (int isub = 0; isub < Ngrids; isub++)
        for (int i = 0; i < qValuesList[isub].size(); i++)
        {
          qValuesList[isub].resize(qValuesList1[isub].size());
        } // for (int isub = 0; isub < Ngrids; isub++)
      qValuesList = qValuesList1;

      flavorsList.resize(Ngrids);
      std::vector<std::vector<int>> flavorsList1 = A1->flavorsList;
      for (int isub = 0; isub < Ngrids; isub++)
        for (int i = 0; i < flavorsList[isub].size(); i++)
        {
          flavorsList[isub].resize(flavorsList1[isub].size());
        } // for (int isub = 0; isub < Ngrids; isub++)
      flavorsList = flavorsList1;
    } // if (op == "multiply")

    if (op == "average")
    {
      // create a vector of LHAGrids from the input files read
      std::vector<LHAGrid *> LHAgrids;
      int Nfiles = inputfiles.size();
      for (int i = 0; i < Nfiles; i++)
      {
        LHAgrids.push_back(new LHAGrid(inputfiles[i]));
      }
      this->CompareLHAGrids(LHAgrids); // check if the grids are compatible
      // If input grids have the same number of grids, then assign the output 
      // grid the appropirate number of subgrids.
      LHAGrid *A1 = LHAgrids[0];
      int Ngrids1 = A1->Ngrids;
      Ngrids = Ngrids1;

      // allocate size to the 2-dimensional PDF values vector
      pdfValuesList.resize(Ngrids);
      for (int i = 0; i < Ngrids; i++)
      {
        int ival1 = A1->pdfValuesList[i].size();
        int ival = ival1;
        pdfValuesList[i].resize(ival);
      }

      // Perform the addition of all corresponding PDF values 
      for (int isub = 0; isub < Ngrids; isub++)
        for (int i = 0; i < pdfValuesList[isub].size(); i++)
          for (int ifile = 0; ifile < Nfiles; ifile++)
            pdfValuesList[isub][i] += LHAgrids[ifile]->pdfValuesList[isub][i];
      
      // divide by the number of files
      for (int isub = 0; isub < Ngrids; isub++)
        for (int i = 0; i < pdfValuesList[isub].size(); i++)
          pdfValuesList[isub][i] /= Nfiles;

      // copy the headers, xValues, qValues, flavor IDs from the first grid to the output grid
      std::vector<std::string> headers1 = A1->headers;
      headers = headers1;

      xValuesList.resize(Ngrids);
      std::vector<std::vector<double>> xValuesList1 = A1->xValuesList;
      for (int isub = 0; isub < Ngrids; isub++)
        for (int i = 0; i < xValuesList[isub].size(); i++)
        {
          xValuesList[isub].resize(xValuesList1[isub].size());
        } // for (int isub = 0; isub < Ngrids; isub++)
      xValuesList = xValuesList1;

      qValuesList.resize(Ngrids);
      std::vector<std::vector<double>> qValuesList1 = A1->qValuesList;
      for (int isub = 0; isub < Ngrids; isub++)
        for (int i = 0; i < qValuesList[isub].size(); i++)
        {
          qValuesList[isub].resize(qValuesList1[isub].size());
        } // for (int isub = 0; isub < Ngrids; isub++)
      qValuesList = qValuesList1;

      flavorsList.resize(Ngrids);
      std::vector<std::vector<int>> flavorsList1 = A1->flavorsList;
      for (int isub = 0; isub < Ngrids; isub++)
        for (int i = 0; i < flavorsList[isub].size(); i++)
        {
          flavorsList[isub].resize(flavorsList1[isub].size());
        } // for (int isub = 0; isub < Ngrids; isub++)
      flavorsList = flavorsList1;
    } // if (op == "average")

  } // LHAGrid(std::string op, std::vector<std::string> inputfiles, std::vector<double> w = std::vector<double>()) : operation(op), files(inputfiles) 

  // Function to read the input grid file
  void ReadLHAGrid(std::string filename)
  {
    std::ifstream inputFile(filename);

    if (!inputFile.is_open())
    {
      std::cerr << "Unable to open file: " << filename << std::endl;
      exit(1);
    }

    std::string line;

    for (int i = 0; i < 2; i++)
    // read the header of the file and store it in the vector headers
    {
      getline(inputFile, line);
      headers.push_back(line);
    }

    getline(inputFile, line); // reads first delimiter "---" after headers.

    while (getline(inputFile, line))
    // while loop will read each subgrid of the file until the end of the file is reached.
    {
      // clear temporary vectors for each iteration.
      xValues.clear();
      qValues.clear();
      flavors.clear();
      pdfValues.clear();

      if (line.empty())
      {
        break;
      } // if (line.empty())
      else
      {
        // if the line after the delimiter "---" is not empty, then it is a new grid
        // and the line is used to assign x values to vector xValues.
        std::istringstream issX(line);
        double valueX;
        while (issX >> valueX)
        {
          xValues.push_back(valueX);
        } // while (issX >> valueX)

        // the next line is read and used to assign q values to vector qValues.
        std::getline(inputFile, line);
        std::istringstream issQ(line);
        double valueQ;
        while (issQ >> valueQ)
        {
          qValues.push_back(valueQ);
        } // while (issQ >> valueQ)

        // the next line is read and used to assign flavor index numbers to vector flavors.
        std::getline(inputFile, line);
        std::istringstream issFlavors(line);
        int valueFlavor;
        while (issFlavors >> valueFlavor)
        {
          flavors.push_back(valueFlavor);
        } // while (issFlavors >> valueFlavor)

        // the block of data after flavor index numbers is read and used to map
        // pdf values to the corresponding flavor index number.
        while (getline(inputFile, line))
        // while loop will read all PDF values until it reaches the delimiter "---".
        {
          if (line == "---")
          {
            break;
          } // if (line == "---")

          else
          {
            std::istringstream issPDF(line);
            double valuePDF;
            int ifla = 0;
            ;
            while (issPDF >> valuePDF)
            {
              pdfValues.push_back(valuePDF);
              ifla++;
            } // while (issPDF >> valuePDF)
            if (ifla != flavors.size())
            // program will exit if the number of flavor indices does not
            // match the number of pdf values.
            {
              std::cout << "Error: number of flavor indices does not match number of pdf values." << std::endl;
              std::cout << "Number of flavor indices: " << flavors.size() << std::endl;
              std::cout << "Number of pdf values: " << ifla << std::endl;
              exit(1);
            } // if (i!=flavors.size())
          } // else (if (line == "---"))
        } // while (getline(file,line)) pdfvalues
      } // else (if line.empty())

      // store vectors of subgrids into vectors of vectors
      xValuesList.push_back(xValues);
      qValuesList.push_back(qValues);
      flavorsList.push_back(flavors);
      pdfValuesList.push_back(pdfValues);

      Ngrids++;
    } // while (getline)

    inputFile.close();
  } // void ReadLHAGrid(std::string filename)

  // Getter function to access Ngrids
  int getNgrids() const {
    return Ngrids;
  }

  // Getter function to access xValueList
  std::vector<std::vector<double>> getxValuesList() const {
    return xValuesList;
  }

  // Getter function to access qValueList
  std::vector<std::vector<double>> getqValuesList() const {
    return qValuesList;
  }

  // Getter function to access flavorsList
  std::vector<std::vector<int>> getflavorsList() const {
    return flavorsList;
  }

  // Getter function to access pdfValuesList
  std::vector<std::vector<double>> getpdfValuesList() const {
    return pdfValuesList;
  }

  void CompareLHAGrids(std::vector<LHAGrid *> &LHAGridsFromFiles)
  // Function used to compare the subgrids of two or more LHAgrids.
  // The function will compare the number of subgridsm the headers of each LHAgrid,
  // the x values, q values, the flavor indices of each subgrid, and the number of pdf values.
  // If any of these do not match, the program will exit.
  {
    int Nfiles = LHAGridsFromFiles.size();
    if (Nfiles < 2)
    {
      std::cout << "Error: need at least two files as input." << std::endl;
      exit(1);
    } // if (Nfiles < 2)

    int Ngridscomp = LHAGridsFromFiles[0]->Ngrids;
    for (int i = 1; i < Nfiles; i++)
    {
      if (LHAGridsFromFiles[i]->Ngrids != Ngridscomp)
      {
        std::cout << "Error: number of subgrids in files do not match." << std::endl;
        exit(1);
      }
    } // for (int i = 1; i < Nfiles; i++)

    // use the values from the first input file to compare subsequent files to.
    std::vector<std::string> headerscomp = LHAGridsFromFiles[0]->headers;
    std::vector<std::vector<double>> xValuesListcomp = LHAGridsFromFiles[0]->xValuesList;
    std::vector<std::vector<double>> qValuesListcomp = LHAGridsFromFiles[0]->qValuesList;
    std::vector<std::vector<int>> flavorsListcomp = LHAGridsFromFiles[0]->flavorsList;
    std::vector<std::vector<double>> pdfValuesListcomp = LHAGridsFromFiles[0]->pdfValuesList;

    for (int i = 1; i < Nfiles; i++)
    {
      if (LHAGridsFromFiles[i]->headers != headerscomp)
      {
        std::cout << "Error: headers for file " << i + 1 << " does not match with first input file." << std::endl;
        exit(1);
      }
      if (LHAGridsFromFiles[i]->xValuesList != xValuesListcomp)
      {
        std::cout << "Error: x values for file " << i + 1 << " does not match with first input file." << std::endl;
        exit(1);
      }
      if (LHAGridsFromFiles[i]->qValuesList != qValuesListcomp)
      {
        std::cout << "Error: q values for file " << i + 1 << " does not match with first input file." << std::endl;
        exit(1);
      }
      if (LHAGridsFromFiles[i]->flavorsList != flavorsListcomp)
      {
        std::cout << "Error: flavor indices for file " << i + 1 << " does not match with first input file." << std::endl;
        exit(1);
      }
      for (int j = 0; j < Ngridscomp; j++)
        if (LHAGridsFromFiles[i]->pdfValuesList[j].size() != pdfValuesListcomp[j].size())
        {
          std::cout << "Error: number of pdf values for file " << i + 1 << " does not match with first input file." << std::endl;
          exit(1);
        }
    } // for (int i = 1; i < Nfiles; i++)
  } // void CompareLHAGrids(std::vector<LHAGrid>)

  //void ConvertPLTGrid(std::string infile, std::string outfile)

  void WriteLHAGrid(std::string outfile)
  {
    std::ofstream file(outfile);
    file << precisionScientific;

    for (int i = 0; i < 2; i++)
    {
      file << headers[i] << std::endl;
    }
    file << "---" << std::endl;

    for (int i = 0; i < Ngrids; i++)
    {
      for (int j = 0; j < xValuesList[i].size(); j++)
      {
        file << xValuesList[i][j] << " ";
      }
      file << std::endl;

      for (int j = 0; j < qValuesList[i].size(); j++)
      {
        file << qValuesList[i][j] << " ";
      }
      file << std::endl;

      for (int j = 0; j < flavorsList[i].size(); j++)
      {
        file << flavorsList[i][j] << " ";
      }
      file << std::endl;

      int M = pdfValuesList[i].size() / flavorsList[i].size();
      int N = flavorsList[i].size();

      for (int j = 0; j < M; j++)
      {
        if (j == 0)
          file << " ";
        for (int k = 0; k < N; k++)
        {
          if (k == 0)
            file << " " << pdfPrecision << pdfValuesList[i][j * N + k];
          else
            file << pdfSpacing << pdfValuesList[i][j * N + k];
          if (k != N - 1)
            file /*<< " "*/;
        }
        file << std::endl;
      }
      if (i != Ngrids - 1)
        file << "---" << std::endl;
      else
        file << "---";

    } // for (int i = 0; i < Ngrids; i++)
    file.close();
  } // void WriteLHAGrid(std::string outfile)

  // destructor
  ~LHAGrid()
  {
    headers.clear();
    xValues.clear();
    qValues.clear();
    flavors.clear();
    pdfValues.clear();

    xValuesList.clear();
    qValuesList.clear();
    flavorsList.clear();
    pdfValuesList.clear();
  } // ~LHAGrid()
}; // class LHAGrid

/* class subgridList
{
private:
  // vectors used to store the data of each grid
    std::vector<std::string> headers;
    std::vector<double> xValues;
    std::vector<double> qValues;
    std::vector<int> flavors;
    std::vector<double> pdfValues; 

    // vectors that will store vectors of each grid
    std::vector<std::vector<double>> xValuesList;
    std::vector<std::vector<double>> qValuesList;
    std::vector<std::vector<int>> flavorsList;
    std::vector<std::vector<double>> pdfValuesList;

    // vectors that will store all sub grid data from each file
    std::vector<std::string> headersListFromFiles;
    std::vector<std::vector<std::vector<double>>> xValuesListFromFiles;
    std::vector<std::vector<std::vector<double>>> qValuesListFromFiles;
    std::vector<std::vector<std::vector<int>>> flavorsListFromFiles;
    std::vector<std::vector<std::vector<double>>> pdfValuesListFromFiles;
    std::vector<int> NgridsListFromFiles;

  int Ngrids; // number of grids in the file

public:
  subgridList()
  {
    Ngrids = 0;
  } // subgridList constructor

   std::vector<LHAGrid> ReadInputGrids(const std::vector<std::string &fileNames>)
  // subgrid::ReadInputGrids takes N files as input. Each file is read and the
  // data from each subgrid is stored in a temporary vector. The temporary vectors
  // are then stored in a collective of vectors that contain all subgrids from one file.
  // This procedure is repeated for each input file and stored in a vector as well.
  {
    int Nfiles = filename.size(); // number of files to be read
    for (const auto &fileName : fileNames)
    // loop over all files to be read
    {

    } // for (const auto &fileName : fileNames)

    // compare headers of all input grids
    for (int i = 0; i < headersListFromFiles.size(); i++)
      for (int j = 0; j < headersListFromFiles[i].size(); j++)
        if (headersListFromFiles[i][j] != headersListFromFiles[0][j])
        {
          std::cout << "Error: headers of input files do not match." << std::endl;
          std::cout << "Header of file " << i << ": " << headersListFromFiles[i][j] << std::endl;
          std::cout << "Header of file 0: " << headersListFromFiles[0][j] << std::endl;
          exit(1);
        } // if (headersListFromFiles[i][j] != headersListFromFiles[0][j])

    // compare number of subgrids of all input grids
    for (int i = 0; i < NgridsListFromFiles.size(); i++)
      if (NgridsListFromFiles[i] != NgridsListFromFiles[0])
      {
        std::cout << "Error: number of grids in input files do not match." << std::endl;
        std::cout << "Number of grids in file " << i << ": " << NgridsListFromFiles[i] << std::endl;
        std::cout << "Number of grids in file 0: " << NgridsListFromFiles[0] << std::endl;
        exit(1);
      } // if (NgridsListFromFiles[i] != NgridsListFromFiles[0])

    // compare x values of all subgrids in all input grids
    for (int i = 0; i < xValuesListFromFiles.size(); i++)
      for (int j = 0; j < xValuesListFromFiles[i].size(); j++)
        for (int k = 0; k < xValuesListFromFiles[i][j].size(); k++)
          if (xValuesListFromFiles[i][j][k] != xValuesListFromFiles[0][j][k])
          {
            std::cout << "Error: x values of input files do not match." << std::endl;
            std::cout << "x value of file " << i << ", grid " << j << ": " << xValuesListFromFiles[i][j][k] << std::endl;
            std::cout << "x value of file 0, grid " << j << ": " << xValuesListFromFiles[0][j][k] << std::endl;
            exit(1);
          } // if (xValuesListFromFiles[i][j][k] != xValuesListFromFiles[0][j][k])

    // compare q values of all subgrids in all input grids
    for (int i = 0; i < qValuesListFromFiles.size(); i++)
      for (int j = 0; j < qValuesListFromFiles[i].size(); j++)
        for (int k = 0; k < qValuesListFromFiles[i][j].size(); k++)
          if (qValuesListFromFiles[i][j][k] != qValuesListFromFiles[0][j][k])
          {
            std::cout << "Error: q values of input files do not match." << std::endl;
            std::cout << "q value of file " << i << ", grid " << j << ": " << qValuesListFromFiles[i][j][k] << std::endl;
            std::cout << "q value of file 0, grid " << j << ": " << qValuesListFromFiles[0][j][k] << std::endl;
            exit(1);
          } // if (qValuesListFromFiles[i][j][k] != qValuesListFromFiles[0][j][k])

    // compare flavors of all subgrids in all input grids
    for (int i = 0; i < flavorsListFromFiles.size(); i++)
      for (int j = 0; j < flavorsListFromFiles[i].size(); j++)
        for (int k = 0; k < flavorsListFromFiles[i][j].size(); k++)
          if (flavorsListFromFiles[i][j][k] != flavorsListFromFiles[0][j][k])
          {
            std::cout << "Error: flavors of input files do not match." << std::endl;
            std::cout << "flavor of file " << i << ", grid " << j << ": " << flavorsListFromFiles[i][j][k] << std::endl;
            std::cout << "flavor of file 0, grid " << j << ": " << flavorsListFromFiles[0][j][k] << std::endl;
            exit(1);
          } // if (flavorsListFromFiles[i][j][k] != flavorsListFromFiles[0][j][k])

  } // subgridList::ReadGrid

  void WriteSubGridsList(std::string outfile)
  {
    std::ofstream file(outfile);
    file << precisionScientific;

    for (int j = 0; j < 2; j++)
    {
      file << headers[j] << std::endl;
    }
    file << "---" << std::endl;

    for (int i = 0; i < Ngrids; i++)
    {
      for (int j = 0; j < xValuesList[i].size(); j++)
      {
        file << xValuesList[i][j] << " ";
      }
      file << std::endl;

      for (int j = 0; j < qValuesList[i].size(); j++)
      {
        file << qValuesList[i][j] << " ";
      }
      file << std::endl;

      for (int j = 0; j < flavorsList[i].size(); j++)
      {
        file << flavorsList[i][j] << " ";
      }
      file << std::endl;

      int M = pdfValuesList[i].size() / flavorsList[i].size();
      int N = flavorsList[i].size();

      for (int j = 0; j < M; j++)
      {
        if (j == 0)
          file << " ";
        for (int k = 0; k < N; k++)
        {
          if (k == 0)
            file << " " << pdfValuesList[i][j * N + k];
          else
            file << pdfValuesList[i][j * N + k];
          if (k != N - 1)
            file << "  ";
        }
        file << std::endl;
      }
      if (i != Ngrids - 1)
        file << "---" << std::endl;
      else
        file << "---";

    } // for (int i = 0; i < Ngrids; i++)
    file.close();
  } // subgridList::PrintGrid

  ~subgridList()
  {

  } // subgridList destructor
};  // class subgridList*/

#endif // SUBGRIDLIST_H
