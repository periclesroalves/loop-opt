#include <stdint.h>

int main(){}
inline int32_t modulo( int32_t a, int32_t b) { return (a+b)%b;}
inline int32_t min( int32_t a, int32_t b) { return a < b ? a : b;}
inline int32_t max( int32_t a, int32_t b) { return a > b ? a : b;}
void dwt_sym8_p_10_u1_1_true_false_true(const int32_t n, const int32_t ndat, const double * __restrict__ x, double * __restrict__ y){
  const int32_t lowfil = -4;
  const int32_t upfil = 3;
  const double sym8_le_fil[] = {
-0.0033824159510050028,
0.03169508781152599,
-0.14329423835127267,
0.4813596512590534,
0.36444189483617895,
-0.027219029917103486,
0.0038087520138944896,
-0.0003029205147241331};
  const double sym8_lo_fil[] = {
-0.0005421323318000107,
0.007607487324976609,
-0.061273359067811076,
0.777185751699628,
-0.0519458381078818,
0.04913717967373029,
-0.014952258337062199,
0.001889950332767689};
  const double sym8_he_fil[] = {
-0.001889950332767689,
0.014952258337062199,
-0.04913717967373029,
0.0519458381078818,
-0.777185751699628,
0.061273359067811076,
-0.007607487324976609,
0.0005421323318000107};
  const double sym8_ho_fil[] = {
-0.0003029205147241331,
0.0038087520138944896,
-0.027219029917103486,
0.36444189483617895,
0.4813596512590534,
-0.14329423835127267,
0.03169508781152599,
-0.0033824159510050028};
  int32_t i1;
  int32_t i2;
  int32_t l;
  double lt1;
  double ht1;
  int32_t mod_arr[(3 - (-4) - (1) - (-4 - (3)) + 1)];
  for (l = -4 - (3); l <= 3 - (-4) - (1); l += 1) {
    mod_arr[l - (-4 - (3))] = modulo(l, n);
  }
#pragma omp parallel default(shared)  private(i1,i2,l,lt1,ht1)
{
#pragma omp for
  for (i2 = 0; i2 <= ndat - (1); i2 += 1) {
    for (i1 = 0; i1 <=  -(-4) - (1); i1 += 1) {
      lt1 = 0;
      ht1 = 0;
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[-4 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.0033824159510050028);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[-4 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.001889950332767689);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[-4 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.0005421323318000107);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[-4 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.0003029205147241331);
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[-3 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.03169508781152599);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[-3 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.014952258337062199);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[-3 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.007607487324976609);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[-3 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.0038087520138944896);
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[-2 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.14329423835127267);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[-2 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.04913717967373029);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[-2 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.061273359067811076);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[-2 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.027219029917103486);
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[-1 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.4813596512590534);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[-1 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.0519458381078818);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[-1 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.777185751699628);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[-1 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.36444189483617895);
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[0 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.36444189483617895);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[0 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.777185751699628);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[0 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.0519458381078818);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[0 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.4813596512590534);
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[1 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.027219029917103486);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[1 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.061273359067811076);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[1 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.04913717967373029);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[1 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.14329423835127267);
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[2 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.0038087520138944896);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[2 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.007607487324976609);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[2 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.014952258337062199);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[2 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.03169508781152599);
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[3 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.0003029205147241331);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[3 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.0005421323318000107);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[3 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.001889950332767689);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[3 + i1 - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.0033824159510050028);
      y[i2 + 0 - (0) + (ndat - (1) - (0) + 1) * (i1 - (0) + (n - (1) - (0) + 1) * (0 - (0)))] = lt1;
      y[i2 + 0 - (0) + (ndat - (1) - (0) + 1) * (i1 - (0) + (n - (1) - (0) + 1) * (1 - (0)))] = ht1;
    }
    for (i1 =  -(-4); i1 <= n - (3) - (1); i1 += 1) {
      lt1 = 0;
      ht1 = 0;
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (-4 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.0033824159510050028);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (-4 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.001889950332767689);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (-4 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.0005421323318000107);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (-4 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.0003029205147241331);
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (-3 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.03169508781152599);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (-3 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.014952258337062199);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (-3 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.007607487324976609);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (-3 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.0038087520138944896);
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (-2 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.14329423835127267);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (-2 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.04913717967373029);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (-2 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.061273359067811076);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (-2 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.027219029917103486);
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (-1 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.4813596512590534);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (-1 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.0519458381078818);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (-1 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.777185751699628);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (-1 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.36444189483617895);
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (0 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.36444189483617895);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (0 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.777185751699628);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (0 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.0519458381078818);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (0 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.4813596512590534);
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (1 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.027219029917103486);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (1 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.061273359067811076);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (1 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.04913717967373029);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (1 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.14329423835127267);
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (2 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.0038087520138944896);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (2 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.007607487324976609);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (2 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.014952258337062199);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (2 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.03169508781152599);
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (3 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.0003029205147241331);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (3 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.0005421323318000107);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (3 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.001889950332767689);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (3 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.0033824159510050028);
      y[i2 + 0 - (0) + (ndat - (1) - (0) + 1) * (i1 - (0) + (n - (1) - (0) + 1) * (0 - (0)))] = lt1;
      y[i2 + 0 - (0) + (ndat - (1) - (0) + 1) * (i1 - (0) + (n - (1) - (0) + 1) * (1 - (0)))] = ht1;
    }
    for (i1 = n - (3); i1 <= n - (1); i1 += 1) {
      lt1 = 0;
      ht1 = 0;
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[-4 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.0033824159510050028);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[-4 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.001889950332767689);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[-4 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.0005421323318000107);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[-4 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.0003029205147241331);
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[-3 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.03169508781152599);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[-3 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.014952258337062199);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[-3 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.007607487324976609);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[-3 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.0038087520138944896);
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[-2 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.14329423835127267);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[-2 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.04913717967373029);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[-2 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.061273359067811076);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[-2 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.027219029917103486);
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[-1 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.4813596512590534);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[-1 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.0519458381078818);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[-1 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.777185751699628);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[-1 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.36444189483617895);
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[0 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.36444189483617895);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[0 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.777185751699628);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[0 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.0519458381078818);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[0 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.4813596512590534);
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[1 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.027219029917103486);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[1 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.061273359067811076);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[1 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.04913717967373029);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[1 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.14329423835127267);
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[2 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.0038087520138944896);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[2 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.007607487324976609);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[2 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.014952258337062199);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[2 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.03169508781152599);
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[3 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.0003029205147241331);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (mod_arr[3 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.0005421323318000107);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[3 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.001889950332767689);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (mod_arr[3 + i1 - (n) - (-4 - (3))] - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.0033824159510050028);
      y[i2 + 0 - (0) + (ndat - (1) - (0) + 1) * (i1 - (0) + (n - (1) - (0) + 1) * (0 - (0)))] = lt1;
      y[i2 + 0 - (0) + (ndat - (1) - (0) + 1) * (i1 - (0) + (n - (1) - (0) + 1) * (1 - (0)))] = ht1;
    }
  }
}
}
void dwt_sym8_fg_10_u1_1_false_true_true(const int32_t n, const int32_t ndat, const double * __restrict__ x, double * __restrict__ y){
  const int32_t lowfil = -4;
  const int32_t upfil = 3;
  const double sym8_le_fil[] = {
-0.0033824159510050028,
0.03169508781152599,
-0.14329423835127267,
0.4813596512590534,
0.36444189483617895,
-0.027219029917103486,
0.0038087520138944896,
-0.0003029205147241331};
  const double sym8_lo_fil[] = {
-0.0005421323318000107,
0.007607487324976609,
-0.061273359067811076,
0.777185751699628,
-0.0519458381078818,
0.04913717967373029,
-0.014952258337062199,
0.001889950332767689};
  const double sym8_he_fil[] = {
-0.001889950332767689,
0.014952258337062199,
-0.04913717967373029,
0.0519458381078818,
-0.777185751699628,
0.061273359067811076,
-0.007607487324976609,
0.0005421323318000107};
  const double sym8_ho_fil[] = {
-0.0003029205147241331,
0.0038087520138944896,
-0.027219029917103486,
0.36444189483617895,
0.4813596512590534,
-0.14329423835127267,
0.03169508781152599,
-0.0033824159510050028};
  int32_t i1;
  int32_t i2;
  int32_t l;
  double lt[(0 - (0) + 1)];
  double ht[(0 - (0) + 1)];
#pragma omp parallel default(shared)  private(i1,i2,l,lt,ht)
{
#pragma omp for
  for (i2 = 0; i2 <= ndat - (1); i2 += 1) {
    for (i1 =  -(3); i1 <=  -(-4) - (1); i1 += 1) {
      lt[0 - (0)] = 0;
      ht[0 - (0)] = 0;
      for (l = max( -(i1), -4); l <= 3; l += 1) {
        lt[0 - (0)] = lt[0 - (0)] + (x[0 - (0) + (1 - (0) + 1) * (l + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (sym8_le_fil[l - (-4)]);
        ht[0 - (0)] = ht[0 - (0)] + (x[0 - (0) + (1 - (0) + 1) * (l + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (sym8_he_fil[l - (-4)]);
        lt[0 - (0)] = lt[0 - (0)] + (x[1 - (0) + (1 - (0) + 1) * (l + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (sym8_lo_fil[l - (-4)]);
        ht[0 - (0)] = ht[0 - (0)] + (x[1 - (0) + (1 - (0) + 1) * (l + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (sym8_ho_fil[l - (-4)]);
      }
      y[i2 + 0 - (0) + (ndat - (1) - (0) + 1) * (i1 - ( -(3)) + (n - (-4) - (1) - ( -(3)) + 1) * (0 - (0)))] = lt[0 - (0)];
      y[i2 + 0 - (0) + (ndat - (1) - (0) + 1) * (i1 - ( -(3)) + (n - (-4) - (1) - ( -(3)) + 1) * (1 - (0)))] = ht[0 - (0)];
    }
    for (i1 =  -(-4); i1 <= n - (3) - (1); i1 += 1) {
      lt[0 - (0)] = 0;
      ht[0 - (0)] = 0;
      lt[0 - (0)] = lt[0 - (0)] + (x[0 - (0) + (1 - (0) + 1) * (-4 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.0033824159510050028);
      ht[0 - (0)] = ht[0 - (0)] + (x[0 - (0) + (1 - (0) + 1) * (-4 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.001889950332767689);
      lt[0 - (0)] = lt[0 - (0)] + (x[1 - (0) + (1 - (0) + 1) * (-4 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.0005421323318000107);
      ht[0 - (0)] = ht[0 - (0)] + (x[1 - (0) + (1 - (0) + 1) * (-4 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.0003029205147241331);
      lt[0 - (0)] = lt[0 - (0)] + (x[0 - (0) + (1 - (0) + 1) * (-3 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.03169508781152599);
      ht[0 - (0)] = ht[0 - (0)] + (x[0 - (0) + (1 - (0) + 1) * (-3 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.014952258337062199);
      lt[0 - (0)] = lt[0 - (0)] + (x[1 - (0) + (1 - (0) + 1) * (-3 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.007607487324976609);
      ht[0 - (0)] = ht[0 - (0)] + (x[1 - (0) + (1 - (0) + 1) * (-3 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.0038087520138944896);
      lt[0 - (0)] = lt[0 - (0)] + (x[0 - (0) + (1 - (0) + 1) * (-2 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.14329423835127267);
      ht[0 - (0)] = ht[0 - (0)] + (x[0 - (0) + (1 - (0) + 1) * (-2 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.04913717967373029);
      lt[0 - (0)] = lt[0 - (0)] + (x[1 - (0) + (1 - (0) + 1) * (-2 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.061273359067811076);
      ht[0 - (0)] = ht[0 - (0)] + (x[1 - (0) + (1 - (0) + 1) * (-2 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.027219029917103486);
      lt[0 - (0)] = lt[0 - (0)] + (x[0 - (0) + (1 - (0) + 1) * (-1 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.4813596512590534);
      ht[0 - (0)] = ht[0 - (0)] + (x[0 - (0) + (1 - (0) + 1) * (-1 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.0519458381078818);
      lt[0 - (0)] = lt[0 - (0)] + (x[1 - (0) + (1 - (0) + 1) * (-1 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.777185751699628);
      ht[0 - (0)] = ht[0 - (0)] + (x[1 - (0) + (1 - (0) + 1) * (-1 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.36444189483617895);
      lt[0 - (0)] = lt[0 - (0)] + (x[0 - (0) + (1 - (0) + 1) * (0 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.36444189483617895);
      ht[0 - (0)] = ht[0 - (0)] + (x[0 - (0) + (1 - (0) + 1) * (0 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.777185751699628);
      lt[0 - (0)] = lt[0 - (0)] + (x[1 - (0) + (1 - (0) + 1) * (0 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.0519458381078818);
      ht[0 - (0)] = ht[0 - (0)] + (x[1 - (0) + (1 - (0) + 1) * (0 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.4813596512590534);
      lt[0 - (0)] = lt[0 - (0)] + (x[0 - (0) + (1 - (0) + 1) * (1 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.027219029917103486);
      ht[0 - (0)] = ht[0 - (0)] + (x[0 - (0) + (1 - (0) + 1) * (1 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.061273359067811076);
      lt[0 - (0)] = lt[0 - (0)] + (x[1 - (0) + (1 - (0) + 1) * (1 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.04913717967373029);
      ht[0 - (0)] = ht[0 - (0)] + (x[1 - (0) + (1 - (0) + 1) * (1 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.14329423835127267);
      lt[0 - (0)] = lt[0 - (0)] + (x[0 - (0) + (1 - (0) + 1) * (2 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.0038087520138944896);
      ht[0 - (0)] = ht[0 - (0)] + (x[0 - (0) + (1 - (0) + 1) * (2 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.007607487324976609);
      lt[0 - (0)] = lt[0 - (0)] + (x[1 - (0) + (1 - (0) + 1) * (2 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.014952258337062199);
      ht[0 - (0)] = ht[0 - (0)] + (x[1 - (0) + (1 - (0) + 1) * (2 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.03169508781152599);
      lt[0 - (0)] = lt[0 - (0)] + (x[0 - (0) + (1 - (0) + 1) * (3 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.0003029205147241331);
      ht[0 - (0)] = ht[0 - (0)] + (x[0 - (0) + (1 - (0) + 1) * (3 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.0005421323318000107);
      lt[0 - (0)] = lt[0 - (0)] + (x[1 - (0) + (1 - (0) + 1) * (3 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (0.001889950332767689);
      ht[0 - (0)] = ht[0 - (0)] + (x[1 - (0) + (1 - (0) + 1) * (3 + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (-0.0033824159510050028);
      y[i2 + 0 - (0) + (ndat - (1) - (0) + 1) * (i1 - ( -(3)) + (n - (-4) - (1) - ( -(3)) + 1) * (0 - (0)))] = lt[0 - (0)];
      y[i2 + 0 - (0) + (ndat - (1) - (0) + 1) * (i1 - ( -(3)) + (n - (-4) - (1) - ( -(3)) + 1) * (1 - (0)))] = ht[0 - (0)];
    }
    for (i1 = n - (3); i1 <= n - (-4) - (1); i1 += 1) {
      lt[0 - (0)] = 0;
      ht[0 - (0)] = 0;
      for (l = -4; l <= min(3, n - (1) - (i1)); l += 1) {
        lt[0 - (0)] = lt[0 - (0)] + (x[0 - (0) + (1 - (0) + 1) * (l + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (sym8_le_fil[l - (-4)]);
        ht[0 - (0)] = ht[0 - (0)] + (x[0 - (0) + (1 - (0) + 1) * (l + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (sym8_he_fil[l - (-4)]);
        lt[0 - (0)] = lt[0 - (0)] + (x[1 - (0) + (1 - (0) + 1) * (l + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (sym8_lo_fil[l - (-4)]);
        ht[0 - (0)] = ht[0 - (0)] + (x[1 - (0) + (1 - (0) + 1) * (l + i1 - (0) + (n - (1) - (0) + 1) * (i2 + 0 - (0)))]) * (sym8_ho_fil[l - (-4)]);
      }
      y[i2 + 0 - (0) + (ndat - (1) - (0) + 1) * (i1 - ( -(3)) + (n - (-4) - (1) - ( -(3)) + 1) * (0 - (0)))] = lt[0 - (0)];
      y[i2 + 0 - (0) + (ndat - (1) - (0) + 1) * (i1 - ( -(3)) + (n - (-4) - (1) - ( -(3)) + 1) * (1 - (0)))] = ht[0 - (0)];
    }
  }
}
}
void dwt_sym8_fs_10_u1_1_false_false_true(const int32_t n, const int32_t ndat, const double * __restrict__ x, double * __restrict__ y){
  const int32_t lowfil = -4;
  const int32_t upfil = 3;
  const double sym8_le_fil[] = {
-0.0033824159510050028,
0.03169508781152599,
-0.14329423835127267,
0.4813596512590534,
0.36444189483617895,
-0.027219029917103486,
0.0038087520138944896,
-0.0003029205147241331};
  const double sym8_lo_fil[] = {
-0.0005421323318000107,
0.007607487324976609,
-0.061273359067811076,
0.777185751699628,
-0.0519458381078818,
0.04913717967373029,
-0.014952258337062199,
0.001889950332767689};
  const double sym8_he_fil[] = {
-0.001889950332767689,
0.014952258337062199,
-0.04913717967373029,
0.0519458381078818,
-0.777185751699628,
0.061273359067811076,
-0.007607487324976609,
0.0005421323318000107};
  const double sym8_ho_fil[] = {
-0.0003029205147241331,
0.0038087520138944896,
-0.027219029917103486,
0.36444189483617895,
0.4813596512590534,
-0.14329423835127267,
0.03169508781152599,
-0.0033824159510050028};
  int32_t i1;
  int32_t i2;
  int32_t l;
  double lt1;
  double ht1;
#pragma omp parallel default(shared)  private(i1,i2,l,lt1,ht1)
{
#pragma omp for
  for (i2 = 0; i2 <= ndat - (1); i2 += 1) {
    for (i1 = 0; i1 <= n - (1); i1 += 1) {
      lt1 = 0;
      ht1 = 0;
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (-4 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (-0.0033824159510050028);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (-4 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (-0.001889950332767689);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (-4 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (-0.0005421323318000107);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (-4 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (-0.0003029205147241331);
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (-3 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (0.03169508781152599);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (-3 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (0.014952258337062199);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (-3 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (0.007607487324976609);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (-3 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (0.0038087520138944896);
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (-2 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (-0.14329423835127267);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (-2 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (-0.04913717967373029);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (-2 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (-0.061273359067811076);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (-2 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (-0.027219029917103486);
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (-1 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (0.4813596512590534);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (-1 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (0.0519458381078818);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (-1 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (0.777185751699628);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (-1 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (0.36444189483617895);
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (0 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (0.36444189483617895);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (0 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (-0.777185751699628);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (0 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (-0.0519458381078818);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (0 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (0.4813596512590534);
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (1 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (-0.027219029917103486);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (1 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (0.061273359067811076);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (1 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (0.04913717967373029);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (1 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (-0.14329423835127267);
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (2 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (0.0038087520138944896);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (2 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (-0.007607487324976609);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (2 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (-0.014952258337062199);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (2 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (0.03169508781152599);
      lt1 = lt1 + (x[0 - (0) + (1 - (0) + 1) * (3 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (-0.0003029205147241331);
      ht1 = ht1 + (x[0 - (0) + (1 - (0) + 1) * (3 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (0.0005421323318000107);
      lt1 = lt1 + (x[1 - (0) + (1 - (0) + 1) * (3 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (0.001889950332767689);
      ht1 = ht1 + (x[1 - (0) + (1 - (0) + 1) * (3 + i1 - (-4) + (n + 3 - (1) - (-4) + 1) * (i2 + 0 - (0)))]) * (-0.0033824159510050028);
      y[i2 + 0 - (0) + (ndat - (1) - (0) + 1) * (i1 - (0) + (n - (1) - (0) + 1) * (0 - (0)))] = lt1;
      y[i2 + 0 - (0) + (ndat - (1) - (0) + 1) * (i1 - (0) + (n - (1) - (0) + 1) * (1 - (0)))] = ht1;
    }
  }
}
}
void dwt_sym8(const int32_t d, const int32_t * n, const int32_t * bc, const double * __restrict__ x, double * __restrict__ y, double * __restrict__ w1, double * __restrict__ w2){
  int32_t i;
  int32_t j;
  int32_t dims_actual[(16 - (0) + 1)];
  int32_t dims_left;
  int32_t ni;
  int32_t ndat;
  int32_t ndat2;
  dims_left = d;
  for (i = 0; i <= d - (1); i += 1) {
    if (bc[i - (0)] == -1) {
      dims_actual[i - (0)] = (n[i - (0)]) * (2) + 16 - (2);
    } else {
      dims_actual[i - (0)] = (n[i - (0)]) * (2);
    }
  }
  if (d == 1) {
    switch (bc[0 - (0)]) {
      case 0 :
        dwt_sym8_p_10_u1_1_true_false_true(n[0 - (0)], 1, x, y);
        break;
      case 1 :
        dwt_sym8_fg_10_u1_1_false_true_true(n[0 - (0)], 1, x, y);
        dims_actual[0 - (0)] = dims_actual[0 - (0)] + 16 - (2);
        break;
      case -1 :
        dwt_sym8_fs_10_u1_1_false_false_true(n[0 - (0)], 1, x, y);
        dims_actual[0 - (0)] = dims_actual[0 - (0)] - (16) + 2;
        break;
      }
  } else {
    ni = n[0 - (0)];
    ndat = 1;
    for (j = 0; j <= d - (1); j += 1) {
      if (j != 0) {
        ndat = (ndat) * (dims_actual[j - (0)]);
      }
    }
    switch (bc[0 - (0)]) {
      case 0 :
        dwt_sym8_p_10_u1_1_true_false_true(ni, ndat, x, w1);
        break;
      case 1 :
        dwt_sym8_fg_10_u1_1_false_true_true(ni, ndat, x, w1);
        dims_actual[0 - (0)] = dims_actual[0 - (0)] + 16 - (2);
        break;
      case -1 :
        dwt_sym8_fs_10_u1_1_false_false_true(ni, ndat, x, w1);
        dims_actual[0 - (0)] = dims_actual[0 - (0)] - (16) + 2;
        break;
      }
    dims_left = dims_left - (1);
    i = 1;
    while (dims_left > 2) {
      ni = n[i - (0)];
      ndat = 1;
      for (j = 0; j <= d - (1); j += 1) {
        if (j != i) {
          ndat = (ndat) * (dims_actual[j - (0)]);
        }
      }
      switch (bc[i - (0)]) {
        case 0 :
          dwt_sym8_p_10_u1_1_true_false_true(ni, ndat, w1, w2);
          break;
        case 1 :
          dwt_sym8_fg_10_u1_1_false_true_true(ni, ndat, w1, w2);
          dims_actual[i - (0)] = dims_actual[i - (0)] + 16 - (2);
          break;
        case -1 :
          dwt_sym8_fs_10_u1_1_false_false_true(ni, ndat, w1, w2);
          dims_actual[i - (0)] = dims_actual[i - (0)] - (16) + 2;
          break;
        }
      i = i + 1;
      ni = n[i - (0)];
      ndat = 1;
      for (j = 0; j <= d - (1); j += 1) {
        if (j != i) {
          ndat = (ndat) * (dims_actual[j - (0)]);
        }
      }
      switch (bc[i - (0)]) {
        case 0 :
          dwt_sym8_p_10_u1_1_true_false_true(ni, ndat, w2, w1);
          break;
        case 1 :
          dwt_sym8_fg_10_u1_1_false_true_true(ni, ndat, w2, w1);
          dims_actual[i - (0)] = dims_actual[i - (0)] + 16 - (2);
          break;
        case -1 :
          dwt_sym8_fs_10_u1_1_false_false_true(ni, ndat, w2, w1);
          dims_actual[i - (0)] = dims_actual[i - (0)] - (16) + 2;
          break;
        }
      i = i + 1;
      dims_left = dims_left - (2);
    }
    if (dims_left == 2) {
      ni = n[i - (0)];
      ndat = 1;
      for (j = 0; j <= d - (1); j += 1) {
        if (j != i) {
          ndat = (ndat) * (dims_actual[j - (0)]);
        }
      }
      switch (bc[i - (0)]) {
        case 0 :
          dwt_sym8_p_10_u1_1_true_false_true(ni, ndat, w1, w2);
          break;
        case 1 :
          dwt_sym8_fg_10_u1_1_false_true_true(ni, ndat, w1, w2);
          dims_actual[i - (0)] = dims_actual[i - (0)] + 16 - (2);
          break;
        case -1 :
          dwt_sym8_fs_10_u1_1_false_false_true(ni, ndat, w1, w2);
          dims_actual[i - (0)] = dims_actual[i - (0)] - (16) + 2;
          break;
        }
      i = i + 1;
      ni = n[i - (0)];
      ndat = 1;
      for (j = 0; j <= d - (1); j += 1) {
        if (j != i) {
          ndat = (ndat) * (dims_actual[j - (0)]);
        }
      }
      switch (bc[i - (0)]) {
        case 0 :
          dwt_sym8_p_10_u1_1_true_false_true(ni, ndat, w2, y);
          break;
        case 1 :
          dwt_sym8_fg_10_u1_1_false_true_true(ni, ndat, w2, y);
          dims_actual[i - (0)] = dims_actual[i - (0)] + 16 - (2);
          break;
        case -1 :
          dwt_sym8_fs_10_u1_1_false_false_true(ni, ndat, w2, y);
          dims_actual[i - (0)] = dims_actual[i - (0)] - (16) + 2;
          break;
        }
    } else {
      ni = n[i - (0)];
      ndat = 1;
      for (j = 0; j <= d - (1); j += 1) {
        if (j != i) {
          ndat = (ndat) * (dims_actual[j - (0)]);
        }
      }
      switch (bc[i - (0)]) {
        case 0 :
          dwt_sym8_p_10_u1_1_true_false_true(ni, ndat, w1, y);
          break;
        case 1 :
          dwt_sym8_fg_10_u1_1_false_true_true(ni, ndat, w1, y);
          dims_actual[i - (0)] = dims_actual[i - (0)] + 16 - (2);
          break;
        case -1 :
          dwt_sym8_fs_10_u1_1_false_false_true(ni, ndat, w1, y);
          dims_actual[i - (0)] = dims_actual[i - (0)] - (16) + 2;
          break;
        }
    }
  }
}
