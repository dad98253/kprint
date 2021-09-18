// kprint.cpp : Defines the entry point for the console application.
//
#pragma comment(linker, "/STACK:16777216")
#include "syscalls.h"

struct deckdata {
	char *deck;
	struct carddata *firstcard;
	struct carddata *lastcard;
};

struct carddata {
	char *card;
	int active;
	int ident;
	int seqnum;
};

struct deckdata decks[MAXDECKS], *ideck=&decks[0];
int numdecks=0;
char *idents[MAXDECKS], *iident=idents[0];
int numidents=0;
struct carddata cards[MAXCARDS], *icard=&cards[0];
int numcards=0;

void error(char*, ...);
void warning(char*, ...);
int threebytei (char a, char b, char c);

int main(int argc, char* argv[])
{
	int f1,f2,n,i,j,ndc,istart,iend,ibstart,ibend,ibistart,ibiend,itemp,linum=0,sword=9,numsync=0,hide=0,eol=0,opl=1,lens,f3,runfound=0;
	int chbflag=0,active=0,wc,seqnum,yankbit=0,csactive=0,identno,lnbc,cc,k,jndc,dl=0,dir=0,decknum=1,identnum=1,ra,compile=0,deckcount=0;
	int newrec=0;
	char buf[BUFSIS], *bufp=&buf[0], *tempc;
	char ascbuf[BUFSIS];
	int masks[] = {1,3,7,15,31,63,126,252};
	int maske[] = {63,126,255-3,255-7,255-15,255-31,255-63,255-127};
	int shiftsl[] = {32,16,8,4,2,1,1,1};
	int shiftsr[] = {1,1,1,1,1,1,2,4};
	int shifter[] = {1,2,4,8,16,32,64,128};
	char dc[] = {':','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
		'0','1','2','3','4','5','6','7','8','9',
		'+','-','*','/','(',')','$','=',' ',',','.','#','[',']','%','"','_','!','&',
		'\x27',
		'?','<','>','@',
		'\x5C',
		'^',';' };
	char ctemp[10] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
	char citemp;
	char recid[] = {40,4,0,0};
	char holdoff[4];
	char prefix[500];
	char lineout[150],linetemp[150];
	char seqfmt[10];
	char runid[] = {28,18,21,14};   // "1RUN" in Display Code

	if (argc < 3)
		error("Usage: kprint from to [dir]");
	if ((f1 = open(argv[1], O_RDONLY | O_BINARY , 0)) == -1)
		error("kprint: can't open %s", argv[1]);
	if ((f2 = creat(argv[2], PERMS)) == -1)
		error("kprint: can't create %s mode %03o", argv[2], PERMS);
	if (argc != 4 )
		warning("kprint: directory for COMPILE files is missing, all files will be created in the current default directory", argv[3]);
	if (argc == 4 && (strlen(strcpy(prefix,argv[3])) != 0 ))
		fprintf(stdout, "\n files will be written to %s", prefix );
	fprintf(stderr, "\n %10.10o ", linum );
	while ((n = read(f1, buf, BUFSIS)) > 0) {
		for (ndc=0; ndc<BUFSIS; ++ndc) ascbuf[ndc]=0;
		ndc=(n*8)/6+1;
		if (ndc > BUFSIS ) error("file too big to convert to short byte format");
		for (i=0; i<ndc; ++i) {
			istart = (i*6)/8;
			iend = (i*6+5)/8;
			ibstart = i*6;
			ibend = i*6+5;
			ibistart = 7 - (ibstart % 8);
			ibiend = 7 - (ibend % 8);
			ascbuf[i] = ((buf[istart] & masks[ibistart])*shiftsl[ibistart])/shiftsr[ibistart];
			if ( istart != iend ) ascbuf[i] = ascbuf[i] + (buf[iend] & maske[ibiend])/shifter[ibiend];
			fprintf(stderr, " %3.2o", ascbuf[i]);
			itemp = ascbuf[i];
			ctemp[i%10] = dc[itemp];
			if ( (i % 10) == 9 ) {
				fprintf(stderr, "    ");
				for (j=0; j<10; ++j) fprintf(stderr, "%c", ctemp[j]);
				linum = linum + 10;
				fprintf(stderr, "\n %10.10o ", linum );
			}
		}
		if (write(f2, ascbuf, ndc) != ndc)
			error("kprint: write error on file %s", argv[2]);
	}

	j=0;
	for (i=0; i<ndc; ++i) {
		if (ascbuf[i] == recid[numsync] && hide == 0) {
				holdoff[numsync] = ascbuf[i];
				numsync++;
			}
			else {
				if (numsync != 0) for (k=0; k<numsync; ++k) {
					buf[j] = holdoff[k];
					j++;
				}
				numsync=0;
			}
		if ( numsync == 0 && hide == 0 ) {
			buf[j] = ascbuf[i];
			j++;
		}
		if (hide != 0) hide--;
		if (numsync == 3) {
			numsync = 0;
			hide = 5;
		}
	}
	jndc = j;
	linum = 0;
	fprintf(stderr, "\n\n ++++++++++++++++++++++++++++  dump blocked data ++++++++++++++++++++++++\n");
	fprintf(stderr, "\n %10.10o ", linum );
	for (i=0; i<jndc; ++i) {
		ascbuf[i] = buf[i];
		fprintf(stderr, " %3.2o", ascbuf[i]);
		itemp = ascbuf[i];
		ctemp[i%10] = dc[itemp];
		if ( (i % 10) == 9 ) {
			fprintf(stderr, "    ");
			for (j=0; j<10; ++j) fprintf(stderr, "%c", ctemp[j]);
			linum = linum + 10;
			fprintf(stderr, "\n %10.10o ", linum );
		}
	}
	ndc=jndc;

	////////////////////// at main
	ideck->firstcard=icard;
	ideck->lastcard=icard;
	i = 0;
	while ( i<ndc ) {
		if (i == 752160 ) {
// trap at line number 222720 oct
//			fprintf(stdout, "at  i = %i \n", i);
		}
		if (opl == 1) {
// we are reading the UPDATE OPL deck records
			fprintf(stdout, "at byte number %10.10o (%i dec)\n", i,i);
			if ( ( ascbuf[i+2] == 0 ) && ( ascbuf[i+3] == 0 ) ) {
//          found an eor mark
				ideck->lastcard=icard-1;
				ideck++;
				numdecks++;
				ideck->firstcard=icard;
				newrec=1;
				if ( ( ascbuf[i+1] & masks[0] ) == 0 ) {
					fprintf(stdout, "found short eor mark\n");
					for (j=0; j<8; ++j) fprintf(stdout, " %3.2o", ascbuf[i+j]);
					fprintf(stdout, "\n");
					i = i + 8;
					continue;
				} 
				else {
					fprintf(stdout, "found long eor mark\n");
					for (j=0; j<10; ++j) fprintf(stdout, " %3.2o", ascbuf[i+j]);
					fprintf(stdout, "\n");
					i = i + 10;
					continue;
				}
			}
//			if ( ( ascbuf[i] & masks[3] ) != 0 ) {
			if ( newrec == 1 && ascbuf[i] == 25 && ascbuf[i+1] == 1 && ascbuf[i+2] == 14 ) {
				fprintf(stdout, "found deck list record\n");
//				fprintf(stdout, "mask = %10.10o , ascbuf = %10.10o\n", masks[4] , ascbuf[i]);
				ideck=&decks[0];
				opl = 0;
				dl = 1;
				continue;
			}
			chbflag = 0;
			if ( (ascbuf[i] & shifter[5]) != 0 ) chbflag = 1;
			active = 0;
			if ( (ascbuf[i] & shifter[4]) != 0 ) active = 1;
			icard->active=active;
			wc = threebytei (ascbuf[i+1],ascbuf[i+2],ascbuf[i+3]);
			seqnum = threebytei (ascbuf[i+4],ascbuf[i+5],ascbuf[i+6]);
			icard->seqnum=seqnum;
			yankbit = 0;
			if ( (ascbuf[i+7] & shifter[5]) != 0 ) yankbit = 1;
			csactive = 0;
			if ( (ascbuf[i+7] & shifter[4]) != 0 ) csactive = 1;
			identno = threebytei ( ( ascbuf[i+7] & masks[3] ),ascbuf[i+8],ascbuf[i+9] );
			icard->ident=identno;
			fprintf(stdout, "found correction history data, last word = %1.1i, activity bit for card = %1.1i\n", chbflag, active);
			fprintf(stdout, "   card word count = %i, sequence number = %i\n", wc, seqnum);
			fprintf(stdout, "   ident number %i , yank bit = %1.1i, c/s activation flag = %1.1i\n", identno, yankbit, csactive);
			i = i + 10;
			while ( chbflag == 0 ) {
				if ( (ascbuf[i] & shifter[5]) != 0 ) chbflag = 1;
				identno = 1;
				j = i + 1;
				while ( identno != 0 ) {
					yankbit = 0;
					if ( (ascbuf[j] & shifter[5]) != 0 ) yankbit = 1;
					csactive = 0;
					if ( (ascbuf[j] & shifter[4]) != 0 ) csactive = 1;
					identno = threebytei ( ( ascbuf[j] & masks[3] ),ascbuf[j+1],ascbuf[j+2] );
					if ( identno != 0 ) {
						fprintf(stdout, "   ident number %i , yank bit = %1.1i, c/s activation flag = %1.1i\n", identno, yankbit, csactive);
						icard->ident=identno;
					}
					j = j + 3;
					if ( j > (i+7) ) break;
				}
				i = i + 10;
			}
			fprintf(stdout, "card = ");
			icard->card=bufp;
			cc = wc * 10;
			lnbc = 0;
			for (j=0; j<cc; ++j) if (ascbuf[i+j] != 0) lnbc = j;
			j = 0;
			while ( j < ( lnbc+1 ) ) {
				if (ascbuf[i+j] == 0) {
					if ( (j+1) > lnbc ) {
						j = j + 1;
						continue;
					}
					if (ascbuf[i+j+1] == 1) {
						fprintf(stdout, ":");
						*bufp++=':';
						j = j + 2;
						continue;
					}
					for ( k=0; k<(ascbuf[i+j+1]+1); ++k) {
						fprintf(stdout, " ");
						*bufp++=' ';
					}
					j = j + 2;
					continue;
				}
				itemp = ascbuf[i+j];
				citemp = dc[itemp];
				fprintf(stdout, "%c", citemp);
				*bufp++ = citemp;
				j = j + 1;
			}
			*bufp++='\0';
			icard++;
			numcards++;
			fprintf(stdout, "\n");
			i = i + cc;
		}
		if (dl == 1 ) {
// we are reading the UPDATE OPL deck list
			fprintf(stdout, "at byte number %10.10o (%i dec)\n", i,i);
			if ( ( ascbuf[i+2] == 0 ) && ( ascbuf[i+3] == 0 ) && threebytei ( ( ascbuf[i] & 0 ),ascbuf[i+4],ascbuf[i+5] ) != 0 && threebytei ( ( ascbuf[i] & 0 ),ascbuf[i],ascbuf[i+1] ) != 0 ) {
//          found an eor mark
				fprintf(stdout, " numdecks = %i, decknum = %i\n", numdecks, decknum);
				if ( (numdecks+1) != decknum ) fprintf(stdout, " ///// WARNING, WILL ROBINSON!... WARNING! Deck count is incorrect!\n");
				if ( ( ascbuf[i+1] & masks[0] ) == 0 ) {
					fprintf(stdout, "found short eor mark\n");
					for (j=0; j<8; ++j) fprintf(stdout, " %3.2o", ascbuf[i+j]);
					fprintf(stdout, "\n");
					i = i + 8;
					dl = 0;
					dir = 1;
					fprintf(stdout, "reading the OPL directory\n");
					continue;
				} 
				else {
					fprintf(stdout, "found long eor mark\n");
					for (j=0; j<10; ++j) fprintf(stdout, " %3.2o", ascbuf[i+j]);
					fprintf(stdout, "\n");
					i = i + 10;
					dl = 0;
					dir = 1;
					fprintf(stdout, "reading the OPL directory\n");
					continue;
				}
			}
			fprintf(stdout, "deck name = ");
			ideck->deck=bufp;
			lnbc = 0;
			for (j=0; j<9; ++j) if (ascbuf[i+j] != 0) lnbc = j;
			j = 0;
			while ( j < ( lnbc+1 ) ) {
				itemp = ascbuf[i+j];
				citemp = dc[itemp];
				fprintf(stdout, "%c", citemp);
				*bufp++=citemp;
				j = j + 1;
			}
			fprintf(stdout, " , deck index number is %i\n", decknum);
			*bufp++='\0';
			ideck++;
			decknum++;
			i = i + 10;
			ra = threebytei ( ( ascbuf[i] & 0 ),ascbuf[i+5],ascbuf[i+6] ) * 4096 + threebytei ( ascbuf[i+7],ascbuf[i+8],ascbuf[i+9] );
			fprintf(stdout, "the random address of the deck record is %10.10o (octal)\n", ra);
			i = i + 10;
		}
		if ( dir == 1 ) {
// we are reading the UPDATE OPL directory record
			fprintf(stdout, "at byte number %10.10o (%i dec)\n", i,i);
			if ( ( ascbuf[i+2] == 0 ) && ( ascbuf[i+3] == 0 ) && threebytei ( ( ascbuf[i] & 0 ),ascbuf[i+4],ascbuf[i+5] ) != 0 && threebytei ( ( ascbuf[i] & 0 ),ascbuf[i],ascbuf[i+1] ) != 0 ) {
//          found an eor mark
				fprintf(stdout, " numidents = %i, identnum = %i\n", numidents, identnum);
				if ( (numidents+1) != identnum ) fprintf(stdout, " ///// WARNING, WILL ROBINSON!... WARNING! Ident count is incorrect!\n");
				if ( ( ascbuf[i+1] & masks[0] ) == 0 ) {
					fprintf(stdout, "found short eor mark\n");
					for (j=0; j<8; ++j) fprintf(stdout, " %3.2o", ascbuf[i+j]);
					fprintf(stdout, "\n");
					i = i + 8;
					dir = 0;
					compile = 1;
					fprintf(stdout, " done reading the OPL directory\n");
					continue;
				} 
				else {
					fprintf(stdout, "found long eor mark\n");
					for (j=0; j<10; ++j) fprintf(stdout, " %3.2o", ascbuf[i+j]);
					fprintf(stdout, "\n");
					i = i + 10;
					dir = 0;
					compile = 1;
					fprintf(stdout, "done reading the OPL directory\n");
					continue;
				}
			}
			fprintf(stdout, "ident name = ");
			idents[numidents]=bufp;                    //////////////////////////////////////////////////////////
			numidents++;
			lnbc = 0;
			for (j=0; j<9; ++j) if (ascbuf[i+j] != 0) lnbc = j;
			j = 0;
			while ( j < ( lnbc+1 ) ) {
				itemp = ascbuf[i+j];
				citemp = dc[itemp];
				fprintf(stdout, "%c", citemp);
				*bufp++=citemp;
				j = j + 1;
			}
			fprintf(stdout, " , ident index number is %i\n", identnum);
			*bufp++='\0';
			identnum++;
			i = i + 10;
		}
		if ( compile == 1 ) {
// dump out the compile files
//			compile = 0;
			ideck=&decks[0];
			while ( deckcount < numdecks )
			{
				deckcount++;
// check for a *COMDECK card
				if ( strncmp(ideck->firstcard->card,"*COMDECK",8) == 0 ) {
					lens = strlen(strcpy(prefix,argv[3]));
					strcat(prefix,"\x5C");
					strcat(prefix,ideck->firstcard->card+9);
					if ( (tempc = strchr(prefix,' ') ) != NULL ) *tempc = '\0';
					if ( strncmp(ideck->firstcard->card+9,"COM",3) == 0 ) strcat(prefix,"x");

					strcat(prefix,".h");
					lens = strlen(prefix);
					if ((f3 = creat(prefix, PERMS)) == -1)
						error("kprint: can't create %s mode %03o", prefix, PERMS);
					for (icard=ideck->firstcard+1; icard<=ideck->lastcard; icard++) {
						if(icard->active == 0 ) continue;
						if ( strncmp(icard->card,"*CALL",5) == 0 ) {
							strncpy(seqfmt,icard->card+6,8);
							if ( (tempc = strchr(seqfmt,' ') ) != NULL ) *tempc = '\0';
							strcpy(linetemp,"      include \"");
							strcat(linetemp,seqfmt);
							strcat(linetemp,".h\"");
							sprintf(lineout,"%-80s",linetemp);
						}
						else {
							strcpy(lineout,icard->card);
						}
						sprintf(seqfmt,"%-8s",idents[icard->ident]);
						strcat(lineout, seqfmt);
//						strcat(lineout,idents[icard->ident]);
						sprintf(seqfmt,"%8i",icard->seqnum);
						strcat(lineout, seqfmt);
						strcat(lineout,"\n");
						if (write(f3, lineout, strlen(lineout)) != strlen(lineout))
							error("kprint: write error on file %s", argv[2]);
					}
//					deckcount++;
					ideck++;
					close(f3);
					continue;
				}
// check for a *DECK card
				if ( strncmp(ideck->firstcard->card,"*DECK",5) == 0 ) {
					lens = strlen(strcpy(prefix,argv[3]));
					strcat(prefix,"\x5C");
					strcat(prefix,ideck->firstcard->card+6);
					if ( (tempc = strchr(prefix,' ') ) != NULL ) *tempc = '\0';
					strcat(prefix,".f");
					lens = strlen(prefix);
					if ((f3 = creat(prefix, PERMS)) == -1)
						error("kprint: can't create %s mode %03o", prefix, PERMS);
					for (icard=ideck->firstcard+1; icard<=ideck->lastcard; icard++) {
						if(icard->active == 0 ) continue;
						if ( strncmp(icard->card,"*CALL",5) == 0 ) {
							strncpy(seqfmt,icard->card+6,8);
							if ( (tempc = strchr(seqfmt,' ') ) != NULL ) *tempc = '\0';
							strcpy(linetemp,"      include \"");
							strcat(linetemp,seqfmt);
							strcat(linetemp,".h\"");
							sprintf(lineout,"%-80s",linetemp);
						}
						else {
							strcpy(linetemp,icard->card);
							sprintf(lineout,"%-80s",linetemp);
						}
						sprintf(seqfmt,"%-8s",idents[icard->ident]);
						strcat(lineout, seqfmt);
//						strcat(lineout,idents[icard->ident]);
						sprintf(seqfmt,"%8i",icard->seqnum);
						strcat(lineout, seqfmt);
						strcat(lineout,"\n");
						if (write(f3, lineout, strlen(lineout)) != strlen(lineout))
							error("kprint: write error on file %s", argv[2]);
					}
//					deckcount++;
					ideck++;
					close(f3);
					continue;
				}
// check for a $$$YANK DECK
				if ( strncmp(ideck->deck,"YANK$$$",7) == 0 ) {
					ideck++;
					continue;
				}
// done checking deck types - end of while on deck count
				ideck++;

			}

			compile = 0;
// skip over the REL and ABS data
			j=i;
			numsync=0;
			while ( i<ndc ){
				if (buf[i] == runid[numsync]) {
					numsync++;
				}
				else {
					numsync=0;
				}
				if (numsync == 4) {
                    // found the start of the RUN compiler ouput file
					i=i-4;
					runfound = 1;
					sword = i%10 ;
					break;
				}
				i++;
			}
			if (runfound == 0 ) i=j;

		}
		if ( opl == 0 && dl == 0 && dir == 0  && compile == 0) {	
// we are reading text data (or REL or ABS files)

			numsync = 0;
			hide = 0;
			if ( ( buf[i+2] == 0 ) && ( buf[i+3] == 0 ) && threebytei ( ( buf[i] & 0 ),buf[i+4],buf[i+5] ) != 0 && threebytei ( ( buf[i] & 0 ),buf[i],buf[i+1] ) != 0 ) {
//          found and eor mark
				if ( ( buf[i+1] & masks[0] ) == 0 ) {
//					fprintf(stdout, "found short eor mark\n");
//					for (j=0; j<8; ++j) fprintf(stdout, " %3.2o", buf[i+j]);
//					fprintf(stdout, "\n");
					i = i + 8;
					continue;
				} 
				else {
//					fprintf(stdout, "found long eor mark\n");
//					for (j=0; j<10; ++j) fprintf(stdout, " %3.2o", buf[i+j]);
//					fprintf(stdout, "\n");
					i = i + 10;
					continue;
				}
			}

//			if (buf[i] == recid[numsync]) {
//				itemp = buf[i];
//				holdoff[numsync] = dc[itemp];
//				numsync++;
//			}
//			else {
//				if (numsync != 0) for (j=0; j<numsync; ++j) fprintf(stdout, "%c", holdoff[j]);
//				numsync=0;
//			}

			itemp = buf[i];
			citemp = dc[itemp];
			if (buf[i] != 0 && numsync == 0 && hide == 0) fprintf(stdout, "%c", citemp);
			if (buf[i] == 0 && numsync == 0 && hide == 0 && eol == 0) for (j=i; j<(i+9); ++j) {
				if ((j+1)%10 == sword && buf[j+1] == 0) {
					eol = 1;
					break;
				}
				if (buf[j] != 0 ) {
					fprintf(stdout, "%c", citemp);
					break;
				}
			}
			if (buf[i] == 0 && buf[i-1] == 0 && i%10 == sword && hide == 0) {
				fprintf(stdout, "\n");
				eol = 0;
			}
			if (hide != 0) hide--;
			if (numsync == 3) {
				numsync = 0;
				hide = 5;
				sword = (sword + 8)%10 ;
			}
			i++;
		}
	}

	return 0;
}

void error(char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	fprintf(stderr, "///// ERROR : ");
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);
	exit(1);

}

void warning(char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	fprintf(stderr, "///// WARNING : ");
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);
	return;

}

int threebytei (char a, char b, char c)
{
	int temp,tempa,tempb,tempc;
	temp = 0;
	tempa = a;
	tempb = b;
	tempc = c;
	temp = a * 512;
	temp = temp + b * 64;
	temp = temp + tempc;
	return temp;
}


