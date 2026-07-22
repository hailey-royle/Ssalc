start procedure[ i64 : argument @@i8 ]
{
	a i64 = 4 + 5;
	b i64 = a + 1;
	c i64 = a + b;
	!return[ c ];
}
