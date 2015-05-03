orbits(mercury, sun).
orbits(venus, sun).
orbits(earth, sun).
orbits(mars, sun).
orbits(moon, earth).
orbits(phobos, mars).
orbits(deimos, mars).

planet(P) :- orbits(P, sun).

satellite(S) :- orbits(S, P), planet(P).
