/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright held by original author
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

\*---------------------------------------------------------------------------*/

#include "SIBS.H"
#include "simpleMatrix.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

void Foam::SIBS::SIMPR
(
    const scalar xStart,
    const scalarField& y,
    const scalarField& dydx,
    const scalarField& dfdx,
    const scalarSquareMatrix& dfdy,
    const scalar deltaX,
    const label nSteps,
    scalarField& yEnd
) const
{
    scalar h = deltaX/nSteps;
    const label nEqns = ode_.nEqns();

    scalarSquareMatrix a(nEqns);
    for (register label i=0; i<nEqns; i++)
    {
        for (register label j=0; j<nEqns; j++)
        {
            a[i][j] = -h*dfdy[i][j];
        }
        ++a[i][i];
    }

    labelList pivotIndices(nEqns);
    LUDecompose(a, pivotIndices);

    for (register label i=0; i<nEqns; i++)
    {
        yEnd[i] = h*(dydx[i] + h*dfdx[i]);
    }

    LUBacksubstitute(a, pivotIndices, yEnd);

    scalarField del(yEnd);
    scalarField ytemp(nEqns);

    for (register label i=0; i<nEqns; i++)
    {
        ytemp[i] = y[i] + del[i];
    }

    scalar x = xStart + h;

    ode_.derivatives(x, ytemp, yEnd);

    for (register label nn=2; nn<=nSteps; nn++)
    {
        for (register label i=0; i<nEqns; i++)
        {
            yEnd[i] = h*yEnd[i] - del[i];
        }

        LUBacksubstitute(a, pivotIndices, yEnd);

        for (register label i=0; i<nEqns; i++)
        {
            ytemp[i] += (del[i] += 2.0*yEnd[i]);
        }

        x += h;

        ode_.derivatives(x, ytemp, yEnd);
    }
    for (register label i=0; i<nEqns; i++)
    {
        yEnd[i] = h*yEnd[i] - del[i];
    }

    LUBacksubstitute(a, pivotIndices, yEnd);

    for (register label i=0; i<nEqns; i++)
    {
        yEnd[i] += ytemp[i];
    }
}


// ************************************************************************* //
