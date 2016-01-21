## Performance Evaluation Correction

Our original evaluation of Errol against the prior work of Grisu3 was
erroneous. The evaluation indicates a 2x speed improvement over Grisu3.
However, corrected performance measurements show a 2x speed loss to Grisu3.

Early on in our research, we wrote benchmarks to test our algorithm in
relation to the previous algorithms Dragon4 and Grisu3. The build system for
Grisu3 available for download did not compile Grisu3 with optimizations turned
on. We thought this may be the case and attempted to configure the Grisu3
build system to on turn compiler optimizations. Unfortunately, we got this
step wrong because of our unfamiliarity with the SCons build system used in
the Grisu3 build. Furthermore, our misconfiguration bug did not cause any
errors, and so we thought that optimizations were already enabled in the
default configuration -- when in reality it was not. The Artifact Evaluation
Committee ran our benchmarks and reproduced our timing numbers, but they
reproduced the same incorrect numbers that we collected. This is not
surprising, since they ran our build script did not turn on optimizations, and
there were no visible errors that would hint at a problem.

The mistake was discovered when Florian Loitsch, author of the Grisu3
algorithm, attempted to replicate our results. He alerted us to the
discrepancy, and we immediately confirmed the corrected results. We then
informed the POPL PC chair of the mistake, and the PC chair advised us to
create a page like this one and update the PDF paper and corresponding talk.

An updated version of the paper will be made available shortly.
