/**
 * Obs: this is NOT a working example, just a snippet!
 */

List ml;
Element* e;

#pragma omp parallel
#pragma omp single
{
  for (e = ml->first; e; e = e->next) {
    // This is so common that, by default, a private variable will be made
    // firstprivate.
    #pragma omp task firstprivate(e)
    process(e);
  }
}
