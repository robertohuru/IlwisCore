#ifndef SAMPLESTATISTICS_H
#define SAMPLESTATISTICS_H

namespace Ilwis {

const int MAXCLASS{250};

struct SampleCell {
    enum Marker{mMEAN=0, mSTANDARDDEV=1};

    SampleCell() : _classMean(0), _classStd(0){}
    double _classMean;
    double _classStd;
};

class SampleStats {
public:
    SampleStats() : _nrOfBands(0) {}
    virtual void prepare(IThematicDomain thematicDomain, quint32 nrOfBands) {}
    virtual void addClass(Raw key) = 0;
    virtual void delClass(Raw key) = 0;
    virtual void resetClass(Raw key) = 0;
    virtual void mergeClass(Raw key1, Raw key2) {}
protected:
    quint32 _nrOfBands;
    IThematicDomain _thematicDomain;
};

class SampleHistogram : public SampleStats{
public:
    void prepare(IThematicDomain thematicDomain, const IRasterCoverage &raster);
    void addClass(Raw key);
    void delClass(Raw key);
    void resetClass(Raw key);
    void mergeClass(Raw key1, Raw key2);

    bool exists(Raw key) const;

    quint32 &at(Raw key, quint32 band, quint32 value);

private:
    std::vector<std::vector<std::vector<quint32>>> _hist;
    IRasterCoverage _raster;
    std::vector<double> _minValuesperBand;
};

class SampleSum : public SampleStats{
public:
    void prepare(IThematicDomain thematicDomain, quint32 nrOfBands);
    void addClass(Raw key);
    void delClass(Raw key);
    void resetClass(Raw key);
    void mergeClass(Raw key1, Raw key2);
    double pixelInClass(Raw key);

    quint32& at(Raw raw, quint32 band1);
private:
    std::vector<std::vector<quint32>> _sums;
};

class SampleSumXY : public SampleStats
{
public:
    void prepare(IThematicDomain thematicDomain, quint32 nrOfBands);
    void addClass(Raw key);
    void delClass(Raw key);
    void resetClass(Raw key);
    void mergeClass(Raw key1, Raw key2);
    quint32& at(Raw key, quint32 band1, quint32 band2);

private:
    std::vector<std::vector<std::vector<quint32>>> _sums;

};

class SampleStatistics: public SampleStats
{
public:
    SampleStatistics();
    void prepare(IThematicDomain thematicDomain, quint32 nrOfBands);
    void addClass(quint32 key);
    void delClass(quint32 key);
    void resetClass(Raw key);
    void mergeClass(quint32 key1, quint32 key2, std::unique_ptr<SampleSum> &sum, std::unique_ptr<SampleSumXY> &sumXY);
    double& at(Raw key, quint32 band, SampleCell::Marker val);

private:
    std::vector<std::vector<SampleCell>> _sampleSpace;

};

typedef std::unique_ptr<SampleStatistics> UPSampleStatistics;
}

#endif // SAMPLESTATISTICS_H
