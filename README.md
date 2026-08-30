# poker-equity (C)

A C99 poker equity and pot-odds calculator: a from-scratch 7-card hand
evaluator, a Chen-formula position-range model, and a Monte Carlo
equity/EV simulator. No dependencies beyond the standard library.

This is a standalone C port of the equity engine behind
[RUNOUT](https://github.com/yourname/runout), a browser-based poker
trainer. Same algorithms, same test benchmarks, different language.

## Build

Requires a C99 compiler (tested with gcc 13) and `make`.

```bash
git clone <this-repo>
cd poker-equity-c
make            # builds ./poker_equity
make test       # builds and runs ./run_tests
```

## Project structure

```
include/
├── deck.h
├── evaluator.h
├── ranges.h
└── equity.h
src/
├── deck.c
├── evaluator.c
├── ranges.c
├── equity.c
└── main.c        CLI entry point
tests/
└── test_evaluator.c   benchmark-based tests
Makefile
```

## Usage

```bash
./poker_equity --hero "As Kd" --board "Th Jd Qc" --pot 40 --bet 20 \
    --opponents 2 --position mp --stack 300 --raise-size pot --fold-pct 35
```

```
$ ./poker_equity --hero "As Kd" --board "Th Jd Qc" --pot 40 --bet 20 --opponents 2 --position mp
Hero:      As Kd
Board:     10h Jd Qc
Opponents: 2 (~18% of hands - still fairly tight)
Equity:    76.9%  (2000 trials)

Pot: $40.0   Bet faced: $20.0   Required equity to call: 25.0%
  EV(fold)                = $0.0
  EV(call)                = $+41.6
  EV(raise to $100, 35.0% fold assumed) = $+76.0

  => RAISE has the highest EV
```

Card notation: rank (`2`-`9`, `T`, `J`, `Q`, `K`, `A`) followed by suit
(`s`, `h`, `d`, `c`), e.g. `As`, `Th`, `7c`.

## Testing

```bash
make test
```

Every assertion is checked against a published or independently-verifiable
benchmark:

```
Equity vs known benchmarks (Monte Carlo, so allow tolerance):
  ✓ AA vs KK, all-in preflop, ~82% (got 0.8219, expected ~0.82)
  ✓ 72o vs AA, all-in preflop, ~12% (got 0.1114, expected ~0.12)
  ✓ 72o vs a random hand, heads-up, ~35% (got 0.3471, expected ~0.34)
  ✓ AA vs 5 random opponents (multiway), ~49% (got 0.5001, expected ~0.49)

9 passed, 0 failed
```

Compiles clean under `-Wall -Wextra`. Uses fixed-capacity arrays throughout
(`CardList` caps at 52 cards, opponents cap at 8) rather than dynamic
allocation — deliberate for a small CLI tool: no heap churn during the
Monte Carlo loop, no `malloc`/`free` pairing to get wrong.

## How the math works

**Equity** is estimated by dealing thousands of random completions of the
board and random opponent hands (respecting the selected position range),
and checking who wins. Hero must beat every opponent; ties split equity
fractionally across everyone tied for best.

**Required equity to call**: `bet / (pot + 2 × bet)`, where `pot` is the pot
*before* the bet being faced.

**Raise EV** is intentionally simplified — either everyone folds (at a rate
you set explicitly) or everyone remaining calls and it goes to showdown. It
does not model partial folds or further raises.

## Known limitations

- Position ranges use the Chen formula, a well-known heuristic — not a
  solved GTO range.
- With more than one opponent, the same position range is applied to all of
  them, for simplicity.
- Required-equity math doesn't account for implied odds from players still
  to act.
- Supports up to 8 simulated opponents (`MAX_OPPONENTS` in `equity.h`);
  raise it there and rebuild if you need more.

## License

MIT — see [LICENSE](LICENSE).
