# ITK Regional Minima Image Filter

Produce a binary image where foreground is the regional minima of the input image.

## Group (Subgroup)

ITKMathematicalMorphology (MathematicalMorphology)

## Description

Regional minima are flat zones surrounded by pixels of greater value.

If the input image is constant, the entire image can be considered as a minima or not. The SetFlatIsMinima() method let the user choose which behavior to use.

## Author

- Gaetan Lehmann. Biologie du Developpement et de la Reproduction, INRA de Jouy-en-Josas, France. 

This class was contributed to the Insight Journal by author Gaetan Lehmann. Biologie du Developpement et de la Reproduction, INRA de Jouy-en-Josas, France. https://www.insight-journal.org/browse/publication/65 

## See Also

- [RegionalMaximaImageFilter](https://itk.org/Doxygen/html/classitk_1_1RegionalMaximaImageFilter.html)

- [ValuedRegionalMinimaImageFilter](https://itk.org/Doxygen/html/classitk_1_1ValuedRegionalMinimaImageFilter.html)

- [HConcaveImageFilter](https://itk.org/Doxygen/html/classitk_1_1HConcaveImageFilter.html)

% Auto generated parameter table will be inserted here

## Example Pipelines

## License & Copyright

Please see the description file distributed with this plugin.

## DREAM3D Mailing Lists

If you need help, need to file a bug report or want to request a new feature, please head over to the [DREAM3DNX-Issues](https://github.com/BlueQuartzSoftware/DREAM3DNX-Issues/discussions) GitHub site where the community of DREAM3D-NX users can help answer your questions.
