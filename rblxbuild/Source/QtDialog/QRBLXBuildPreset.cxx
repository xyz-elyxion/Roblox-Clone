/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */
#include "QRBLXBuildPreset.h"

bool operator==(QRBLXBuildPreset const& lhs, QRBLXBuildPreset const& rhs)
{
  return lhs.name == rhs.name && lhs.displayName == rhs.displayName &&
    lhs.description == rhs.description && lhs.generator == rhs.generator &&
    lhs.architecture == rhs.architecture &&
    lhs.setArchitecture == rhs.setArchitecture && lhs.toolset == rhs.toolset &&
    lhs.setToolset == rhs.setToolset && lhs.enabled == rhs.enabled;
}

bool operator!=(QRBLXBuildPreset const& lhs, QRBLXBuildPreset const& rhs)
{
  return !(lhs == rhs);
}

bool operator<(QRBLXBuildPreset const& lhs, QRBLXBuildPreset const& rhs)
{
  return lhs.name < rhs.name ||
    (lhs.name == rhs.name &&
     (lhs.displayName < rhs.displayName ||
      (lhs.displayName == rhs.displayName &&
       (lhs.description < rhs.description ||
        (lhs.description == rhs.description &&
         (lhs.generator < rhs.generator ||
          (lhs.generator == rhs.generator &&
           (lhs.architecture < rhs.architecture ||
            (lhs.architecture == rhs.architecture &&
             (lhs.setArchitecture < rhs.setArchitecture ||
              (lhs.setArchitecture == rhs.setArchitecture &&
               (lhs.toolset < rhs.toolset ||
                (lhs.toolset == rhs.toolset &&
                 (lhs.setToolset < rhs.setToolset ||
                  (lhs.setToolset == rhs.setToolset &&
                   (lhs.enabled < rhs.enabled))))))))))))))));
}

bool operator<=(QRBLXBuildPreset const& lhs, QRBLXBuildPreset const& rhs)
{
  return rhs >= lhs;
}

bool operator>(QRBLXBuildPreset const& lhs, QRBLXBuildPreset const& rhs)
{
  return rhs < lhs;
}

bool operator>=(QRBLXBuildPreset const& lhs, QRBLXBuildPreset const& rhs)
{
  return !(lhs < rhs);
}
