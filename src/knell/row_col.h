#ifndef KN_ROW_COL_H
#define KN_ROW_COL_H

/**
 * Provide a separate (row, column) type to indicate position, which has a
 * different semantic ordering than 2D vectors with (x, y).  This prevents
 * accidentally plugging row -> x and column -> y, by allowing explicitness for
 * coordinate usage.
 */
typedef struct {
	uint32_t row, col;
} RowColu32;

#endif /* KN_ROW_COL_H */
