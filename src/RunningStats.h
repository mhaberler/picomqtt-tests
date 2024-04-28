#ifndef RUNNINGSTATS_H
#define RUNNINGSTATS_H
// from: https://www.johndcook.com/blog/skewness_kurtosis/

typedef enum {
  CI90=0,
  CI95,
  CI99,
} ci_t;

class RunningStats {
public:
  RunningStats();
  void Clear();
  void Push(double x);
  long long NumDataValues() const;
  double Mean() const;
  double Variance() const;
  double StandardDeviation() const;
  double Skewness() const;
  double Kurtosis() const;
  double ConfidenceInterval(ci_t ci);

  friend RunningStats operator+( RunningStats const &a,  RunningStats const &b);
  RunningStats &operator+=(const RunningStats &rhs);

private:
  long long n;
  double M1, M2, M3, M4;
};

#endif
