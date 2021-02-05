#ifndef REVIEWS_H
# define REVIEWS_H

# include "gointer.h"
# include "connect.h"

typedef struct _Game Review;
typedef enum {
    /* Global properties */
    retSIZE, retNODENAME, retKOMI, retHANDICAP, retENTEREDBY, retCOPYRIGHT,
    retPLACE, retDATE, retRESULT, retTOURNAMENT, retNAME, retGAME,
    retWHITESTRENGTH, retBLACKSTRENGTH, retBLACKNAME, retWHITENAME,
    /* Local properties */
    retBLACK, retWHITE, retBLACKSET, retWHITESET, retEMPTYSET, retCOMMENT,
    retBLACKTIME, retWHITETIME, retLETTERS
} ReviewEntryType;

extern Widget ReviewsButton;
extern void InitReviews(Widget Toplevel);
extern void CleanReviews(void);
extern void SgfList(const NameList *Sgfs);
extern void ReviewList(const NameList *Reviews);
extern void ReviewStart(const char *Name);
extern void ReviewEntryBegin(int Nr);
extern void ReviewNewNode(void);
extern void ReviewOpenVariation(void);
extern void ReviewCloseVariation(void);
extern void ReviewGlobalProperty(int Name, const char *Value);
extern void ReviewLocalProperty(int Name, const NameList *Value);
extern void ReviewEnd(int Last);
extern void ReviewStop(void);
extern void UnReview(Connection conn);
extern void ReviewsTime(unsigned long diff);
extern void ReviewNotFound(void);
extern void ReviewListWanted(Connection conn, int Type);
extern void ReviewWanted(Connection conn, const char *Name, int Type);
extern void CheckReview(Connection conn);
extern void TestDeleteReview(Review *review);
extern void DumpReviews(const char *args);
#endif /* REVIEWS_H */
