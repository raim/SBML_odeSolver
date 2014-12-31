## 
## This library is free software; you can redistribute it and/or modify it
## under the terms of the GNU Lesser General Public License as published
## by the Free Software Foundation; either version 2.1 of the License, or
## any later version.
## 
## This library is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY, WITHOUT EVEN THE IMPLIED WARRANTY OF
## MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. The software and
## documentation provided hereunder is on an "as is" basis, and the
## authors have no obligations to provide maintenance, support,
## updates, enhancements or modifications.  In no event shall the
## authors be liable to any party for direct, indirect, special,
## incidental or consequential damages, including lost profits, arising
## out of the use of this software and its documentation, even if the
## authors have been advised of the possibility of such damage.  See
## the GNU Lesser General Public License for more details.
## 
## You should have received a copy of the GNU Lesser General Public License
## along with this library; if not, write to the Free Software Foundation,
## Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
## 
AM_CPPFLAGS = -I$(srcdir)/../src @SBML_CPPFLAGS@ @SUNDIALS_CPPFLAGS@
AM_CFLAGS = -Wno-unknown-pragmas -Wall -Wextra -ansi -std=iso9899:1990
AM_LDFLAGS = @SBML_LDFLAGS@ @SUNDIALS_LDFLAGS@ @GRACE_LDFLAGS@ @GRAPHVIZ_LDFLAGS@
LDADD = ../src/libODES.la @SBML_LIBS@ @SUNDIALS_LIBS@ @GRACE_LIBS@ @GRAPHVIZ_LIBS@

noinst_PROGRAMS = evaluateast integrate integratorinstance odemodel setvariablevalue
evaluateast_SOURCES = evaluateast.c
integrate_SOURCES = integrate.c
integratorinstance_SOURCES = integratorinstance.c
odemodel_SOURCES = odemodel.c
setvariablevalue_SOURCES = setvariablevalue.c
