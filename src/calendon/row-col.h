#ifndef CN_ROW_COL_H
#define CN_ROW_COL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Provide a separate (row, column) type to indicate position, which has a
 * different semantic ordering than 2D vectors with (x, y).  This prevents
 * accidentally plugging row -> x and column -> y, by allowing explicitness for
 * coordinate usage.
 */
typedef struct {
	uint32_t row, col;
} CnRowColu32;

#ifdef __cplusplus
}
#endif

#endif /* CN_ROW_COL_H */
